# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged node_mgr_media into the media build.
# (Named *_media to avoid colliding with the Zephyr-side node_mgr in nn-modules.)
# Source REUSED from nn-modules (submodule in a standalone clone, else monorepo).
foreach(_c "$ENV{NN_MODULES_DIR}/libs/node_mgr_media"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/node_mgr_media"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/node_mgr_media")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "node_mgr_media: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
idf_component_register(
    SRCS "${LIB}/src/node_mgr.c"
    INCLUDE_DIRS "${LIB}/include"
    REQUIRES "nn_registry" "nn_link" "nn_p4ctl" "nn_netstream" "nn_prov" "nn_ctrl" "nn_osal"
)
