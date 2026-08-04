/* SPDX-License-Identifier: Apache-2.0 */
/*
 * nn-app-media-network — ESP32-C6 networking co-processor (media-network-1).
 *
 * Milestone 1 (CLI only): on boot, power-cycle the P4 and come up as the
 * SDIO slave, then wait for the P4 master to establish the link.  A console
 * (REPL) exposes `p4 ...` (boot/reset control) and `link-*` (serial-like
 * data) commands.  Wi-Fi/BLE are declared by the registry but not yet
 * exercised at this milestone.
 */
#include "node_mgr_media/node_mgr_media.h"
#include "nn_link/nn_link.h"
#include "nn_link/nn_video.h"
#include "nn_link/nn_audio.h"
#include "nn_link/nn_time.h"
#include "nn_link/nn_ota_link.h"
#include "nn_netstream/nn_netstream.h"
#include "nn_ota/nn_ota_c6.h"
#include "nn_timesync/nn_timesync.h"
#include "nn_registry/nn_features.h"

#include "esp_console.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "media-network";

/* ── A/V uplink (P4 -> C6 -> host), DIRECT RELAY ──────────────────────────
 * The C6 does NOT reorder (that moved to the host, which has memory): each
 * incoming nn_link packet is a video FRAGMENT or one AUDIO (AAC) frame; the C6
 * rewrites the P4-local timestamp into ABSOLUTE hub-epoch ms (nn_timesync) and
 * emits it immediately as a typed uplink record.  The host's A/V-sync element
 * orders by timestamp before RTP.  (Removing the on-chip reorder pool also
 * freed the heap that was starving nn_sectun's AES — see git history.) */

/* stats */
static uint32_t s_vframes, s_vkeys, s_vbytes, s_vdrops;
static uint32_t s_aframes, s_abytes;
static int64_t  s_vt0;
static int      s_vcur_seq = -1;
static uint16_t s_vnext_frag;

/* Cached H.264 parameter sets (SPS=NAL7, PPS=NAL8): re-injected before every
 * keyframe so a host joining mid-stream can decode. */
static uint8_t  s_sps[96]; static size_t s_sps_len;
static uint8_t  s_pps[96]; static size_t s_pps_len;

static int scan_cache_params(const uint8_t *d, size_t n)
{
    int first = -1;
    for (size_t i = 0; i + 4 < n; ) {
        if (!(d[i] == 0 && d[i+1] == 0 && d[i+2] == 1)) { i++; continue; }
        size_t nal = i + 3;
        int type = d[nal] & 0x1f;
        if (first < 0) first = type;
        size_t j = nal + 1;
        while (j + 3 < n && !(d[j] == 0 && d[j+1] == 0 && d[j+2] == 1)) j++;
        size_t end = (j + 3 < n) ? j : n;
        size_t sc = i;
        if (sc > 0 && d[sc-1] == 0) sc--;
        size_t len = end - sc;
        if (type == 7 && len <= sizeof s_sps) { memcpy(s_sps, d + sc, len); s_sps_len = len; }
        else if (type == 8 && len <= sizeof s_pps) { memcpy(s_pps, d + sc, len); s_pps_len = len; }
        i = end;
    }
    return first;
}

static void on_video_packet(const uint8_t *data, size_t len)
{
    if (len < (size_t)NN_VID_HDR_LEN) { s_vdrops++; return; }
    const nn_vid_hdr_t *h = (const nn_vid_hdr_t *)data;
    const uint8_t *pay = data + NN_VID_HDR_LEN;
    size_t plen = len - NN_VID_HDR_LEN;
    bool key = (h->flags & NN_VID_FLAG_KEY) != 0;
    uint64_t ts = nn_timesync_p4_to_abs(h->ts_ms);    /* P4-local -> absolute */

    if (h->flags & NN_VID_FLAG_START) { s_vcur_seq = h->seq; s_vnext_frag = 0; }
    if (h->seq != s_vcur_seq || h->frag != s_vnext_frag) {   /* lost a fragment */
        if (s_vcur_seq >= 0) s_vdrops++;
        s_vcur_seq = -1;
        return;
    }
    s_vnext_frag++;

    /* First fragment: cache SPS/PPS; re-inject before a keyframe lacking them. */
    if (h->flags & NN_VID_FLAG_START) {
        int first = scan_cache_params(pay, plen);
        if (key && first != 7 && s_sps_len && s_pps_len) {
            nn_netstream_send_record(NN_REC_VIDEO, NN_VID_FLAG_START, h->seq, ts, s_sps, s_sps_len);
            nn_netstream_send_record(NN_REC_VIDEO, 0, h->seq, ts, s_pps, s_pps_len);
        }
    }

    nn_netstream_send_record(NN_REC_VIDEO, h->flags, h->seq, ts, pay, plen);
    s_vbytes += plen;

    if (h->flags & NN_VID_FLAG_END) {
        if (s_vt0 == 0) s_vt0 = esp_timer_get_time();
        s_vframes++;
        if (key) s_vkeys++;
        if ((s_vframes % 150) == 0) {
            int64_t dt = esp_timer_get_time() - s_vt0;
            float fps = dt > 0 ? s_vframes * 1e6f / dt : 0;
            ESP_LOGI(TAG, "video: %lu frames (%lu key) %lu KB, %.1f fps, drops=%lu",
                     (unsigned long)s_vframes, (unsigned long)s_vkeys,
                     (unsigned long)(s_vbytes / 1024), fps, (unsigned long)s_vdrops);
        }
        s_vcur_seq = -1;
    }
}

static void on_audio_packet(const uint8_t *data, size_t len)
{
    if (len < (size_t)NN_AUD_HDR_LEN) return;
    const nn_aud_hdr_t *h = (const nn_aud_hdr_t *)data;
    const uint8_t *pay = data + NN_AUD_HDR_LEN;
    size_t plen = len - NN_AUD_HDR_LEN;
    if (plen == 0) return;
    uint64_t ts = nn_timesync_p4_to_abs(h->ts_ms);
    nn_netstream_send_record(NN_REC_AUDIO, h->flags, h->seq, ts, pay, plen);
    s_aframes++; s_abytes += plen;
}

static int cmd_vid(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("video rx: frames=%lu key=%lu bytes=%lu drops=%lu\n",
           (unsigned long)s_vframes, (unsigned long)s_vkeys,
           (unsigned long)s_vbytes, (unsigned long)s_vdrops);
    printf("audio rx: frames=%lu bytes=%lu\n",
           (unsigned long)s_aframes, (unsigned long)s_abytes);
    char ts[140]; nn_timesync_status(ts, sizeof ts);
    printf("time    : %s\n", ts);
    char ns[200]; nn_netstream_status(ns, sizeof ns);
    printf("net     : %s\n", ns);
    return 0;
}

/* Route incoming link packets: video / audio / time / OTA control / plain text. */
static void on_peer_rx(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (len >= 1 && data[0] == NN_VID_MAGIC)  { on_video_packet(data, len); return; }
    if (len >= 1 && data[0] == NN_AUD_MAGIC)  { on_audio_packet(data, len); return; }
    if (len >= 1 && data[0] == NN_TIME_MAGIC) { nn_timesync_on_p4_msg(data, len); return; }
    if (len >= 1 && data[0] == NN_OTA_MAGIC)  { nn_ota_c6_on_p4_msg(data, len); return; }
    printf("\n[P4 -> C6, %u bytes]: ", (unsigned)len);
    fwrite(data, 1, len, stdout);
    printf("\n");
    fflush(stdout);
}

void app_main(void)
{
    /* NVS is needed later (wifi/ble provisioning); init early & harmlessly. */
    esp_err_t nret = nvs_flash_init();
    if (nret == ESP_ERR_NVS_NO_FREE_PAGES || nret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_LOGI(TAG, "boot: %s", nn_registry_summary());

    nn_timesync_init();              /* absolute clock + P4 offset (hub-driven) */
    nn_link_set_rx_cb(on_peer_rx, NULL);

    /* OTA orchestrator: reconcile armed state on boot (self-confirm a swap) +
     * set up P4<->C6 OTA round-trip primitives.  The hub drives the rest over
     * the nn_ctrl control channel. */
    nn_ota_c6_init();

    /* Console first so it's responsive while the link comes up. */
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_cfg.prompt = "media-net>";
    esp_console_dev_uart_config_t uart_cfg = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_cfg, &repl_cfg, &repl));
    esp_console_register_help_command();

    /* node_mgr wires the SDIO link, P4 control, the Wi-Fi/TCP video uplink
     * (nn_netstream) and BLE provisioning (nn_prov), and registers their
     * console commands. */
    ESP_ERROR_CHECK(node_mgr_init());
    const esp_console_cmd_t vcmd = { .command = "vid", .help = "Incoming video stream stats", .func = cmd_vid };
    esp_console_cmd_register(&vcmd);

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    /* Power-cycle the P4, start the SDIO slave, and either restore the Wi-Fi
     * uplink (if provisioned — BLE radio stays off, no coexistence) or bring
     * BLE up to advertise for provisioning (if not). */
    ESP_ERROR_CHECK(node_mgr_start());

    ESP_LOGI(TAG, "ready. Provision over BLE (hub), or 'net wifi <ssid> <pass>' + 'net start'");
}
