# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_camera into the media build.  Source is
# REUSED from nn-modules (submodule in a standalone app clone, else the monorepo
# device/modules/libs path).  First existing path wins.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_camera"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_camera"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_camera")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "nn_camera: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
set(srcs "")
set(reqs "nn_registry" "nn_osal")
if(IDF_TARGET STREQUAL "esp32p4")
    set(srcs "${LIB}/src/nn_camera.c" "${LIB}/src/nn_camera_cli.c")
    list(APPEND reqs esp_video esp_driver_i2c esp_timer console esp_psram)
endif()
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${LIB}/include"
    REQUIRES ${reqs}
)
