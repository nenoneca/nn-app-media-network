# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_ota into the media build.  Source REUSED
# from nn-modules (submodule in a standalone clone, else monorepo path).
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_ota"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_ota"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_ota")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "nn_ota: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
idf_component_register(
    SRCS "${LIB}/src/nn_ota_dfu.c" "${LIB}/src/nn_ota_p4.c" "${LIB}/src/nn_ota_c6.c"
    INCLUDE_DIRS "${LIB}/include"
    REQUIRES app_update esp_partition esp_app_format mbedtls nn_link nn_osal nn_pal
    PRIV_REQUIRES spi_flash esp_system nvs_flash
)
