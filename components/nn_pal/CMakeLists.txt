# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_pal (esp_idf backend) into the media
# build.  Source is REUSED, not inlined: a standalone app clone vendors it via
# the nn-modules submodule (./nn-modules/modules/libs/nn_pal); the monorepo uses
# device/modules/libs/nn_pal.  First existing path wins.
#
# Backends are added as the corresponding media component is converged:
#   dfu  (nn_ota)        — P4   [done]
#   wifi (nn_netstream)  — P5
#   ble  (nn_prov)       — P6
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_pal"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_pal"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_pal")
    if(EXISTS "${_c}")
        set(PAL "${_c}")
        break()
    endif()
endforeach()
if(NOT PAL)
    message(FATAL_ERROR "nn_pal: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()

# dfu (esp_ota) builds on every target.  wifi/ble only exist on a radio chip:
# SRCS are read in the build pass (CONFIG_* valid) so they gate on the feature;
# REQUIRES are resolved at early expansion (CONFIG_* empty), so the radio
# components (esp_wifi/bt) are keyed on IDF_TARGET — they only exist on the C6.
set(srcs "${PAL}/src/esp_idf/dfu.c")
if(CONFIG_NN_FEATURE_WIFI)
    list(APPEND srcs "${PAL}/src/esp_idf/wifi.c")
endif()
if(CONFIG_NN_FEATURE_BLE)
    list(APPEND srcs "${PAL}/src/esp_idf/ble.c")
endif()

set(reqs app_update esp_partition nn_osal)
if(IDF_TARGET STREQUAL "esp32c6")      # the only radio chip in the media system
    list(APPEND reqs esp_wifi esp_netif esp_event bt)
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${PAL}/include"
    REQUIRES ${reqs}
)
target_compile_definitions(${COMPONENT_LIB} PUBLIC CONFIG_NN_PAL_BACKEND_ESP_IDF=1)
