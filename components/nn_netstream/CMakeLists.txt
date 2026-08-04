# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_netstream into the media build.  Source REUSED
# from nn-modules (submodule in a standalone clone, else monorepo path).
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_netstream"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_netstream"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_netstream")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "nn_netstream: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
set(srcs "")
if(CONFIG_NN_FEATURE_WIFI)
    set(srcs "${LIB}/src/nn_netstream.c" "${LIB}/src/nn_netstream_cli.c")
endif()
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${LIB}/include"
    REQUIRES "esp_wifi" "esp_netif" "esp_event" "nvs_flash" "lwip" "console" "nn_registry" "nn_sectun" "nn_osal" "nn_pal"
)
