# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_osal (esp_idf backend) into the
# media build.  The source is REUSED, not inlined: a standalone app clone
# vendors it via the nn-modules submodule (./nn-modules/modules/libs/nn_osal);
# the monorepo layout resolves relatively/nn_osal.  First existing path wins.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_osal"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_osal"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_osal")
    if(EXISTS "${_c}")
        set(OSAL "${_c}")
        break()
    endif()
endforeach()
if(NOT OSAL)
    message(FATAL_ERROR "nn_osal: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
idf_component_register(
    SRCS
        "${OSAL}/src/esp_idf/log.c"   "${OSAL}/src/esp_idf/sync.c"
        "${OSAL}/src/esp_idf/thread.c" "${OSAL}/src/esp_idf/time.c"
        "${OSAL}/src/esp_idf/system.c" "${OSAL}/src/esp_idf/socket.c"
        "${OSAL}/src/esp_idf/storage.c" "${OSAL}/src/esp_idf/gpio.c"
    INCLUDE_DIRS "${OSAL}/include"
    REQUIRES nvs_flash lwip esp_timer esp_app_format esp_system esp_rom esp_driver_gpio
)
target_compile_definitions(${COMPONENT_LIB} PUBLIC CONFIG_NN_OSAL_BACKEND_ESP_IDF=1)
