# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_audio into the media build.  Source is
# REUSED from nn-modules (submodule in a standalone app clone, else the monorepo
# device/modules/libs path).  First existing path wins.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_audio"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_audio"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_audio")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "nn_audio: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
set(srcs "")
if(CONFIG_NN_FEATURE_CAMERA)
    set(srcs "${LIB}/src/nn_audio.c")
endif()
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${LIB}/include"
    REQUIRES nn_link esp_driver_i2s esp_driver_i2c esp_timer
             esp_audio_codec esp_codec_dev nn_osal
    PRIV_REQUIRES console
)
