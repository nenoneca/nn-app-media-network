# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_p4ctl into the media build.  Source is
# REUSED from nn-modules (submodule in a standalone app clone, else the monorepo
# device/modules/libs path).  First existing path wins.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_p4ctl"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_p4ctl"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_p4ctl")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "nn_p4ctl: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
set(srcs "")
if(CONFIG_NN_FEATURE_P4CTL)
    set(srcs "${LIB}/src/nn_p4ctl.c" "${LIB}/src/nn_p4ctl_cli.c")
endif()
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${LIB}/include"
    REQUIRES "esp_driver_gpio" "console" "nn_osal"
)
