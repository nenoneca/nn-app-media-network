# SPDX-License-Identifier: Apache-2.0
idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES "node_mgr_media" "nn_link" "nn_p4ctl" "nn_registry" "console" "esp_driver_gpio" "nvs_flash" "esp_timer" "nn_netstream" "nn_ota" "nn_timesync"
)
