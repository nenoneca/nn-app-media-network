# nn-app-media-network (ESP32-C6 · media-network-1)

The **networking co-processor** of the nn media camera system — the ESP32-C6.
NCP-Host style: it owns Wi-Fi + BLE, drives the ESP32-P4's boot/reset, and is
the SDIO **bus slave**. It pairs with the P4 camera host
([`nn-app-media`](https://github.com/chalos/nn-app-media)). OpenThread is
excluded from the whole system (the C6 reaches the hub / video-stream server
over Wi-Fi).

## Build & flash (ESP-IDF v6.0.1)

```bash
. <path-to>/esp-idf-v6.0.1/export.sh           # see chalos/nn-esp-idf
idf.py set-target esp32c6 build
# CH343 UART port (auto-reset works on this board):
idf.py -p /dev/ttyACM11 flash monitor
```

## SDIO wiring (C6 slave, fixed IO_MUX pins)

| CLK | CMD | D0 | D1 | D2 | D3 |
|-----|-----|----|----|----|----|
| GPIO19 | GPIO18 | GPIO20 | GPIO21 | GPIO22 | GPIO23 |

P4 control: C6 **GPIO6 → P4 GPIO35** (BOOT), C6 **GPIO5 → P4 EN/RESET**.

On boot the C6 starts the SDIO slave (listening) and then power-cycles the P4 so
it initiates the link. Shared components live in `components/` (vendored from the
`media/` monorepo); see the monorepo README for full architecture + bring-up
notes (external pull-ups on CMD/D0–D3 are required for reliable data transfer).

## CLI (Milestone 1)

`media-net>` prompt: `p4 power-cycle|download|reset|hold|release`,
`link-wait [ms]`, `link-send <text>`, `link-status`.
