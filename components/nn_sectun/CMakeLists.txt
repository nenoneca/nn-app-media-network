# SPDX-License-Identifier: Apache-2.0
# CONVERGED nn_sectun (over nn_osal + nn_crypto).  Source REUSED, not inlined:
# standalone app clone -> nn-modules submodule; monorepo -> device/modules/libs.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_sectun"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_sectun"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_sectun")
    if(EXISTS "${_c}")
        set(NS "${_c}")
        break()
    endif()
endforeach()
if(NOT NS)
    message(FATAL_ERROR "nn_sectun: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
set(srcs "")
if(CONFIG_NN_FEATURE_WIFI AND CONFIG_NN_FEATURE_BLE)
    set(srcs "${NS}/src/nn_sectun.c")
endif()
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${NS}/include"
    REQUIRES nn_osal nn_crypto
)
