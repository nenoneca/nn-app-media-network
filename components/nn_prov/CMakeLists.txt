# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_prov into the media build.  Source REUSED
# from nn-modules (submodule in a standalone clone, else monorepo path).
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_prov"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_prov"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_prov")
    if(EXISTS "${_c}")
        set(LIB "${_c}")
        break()
    endif()
endforeach()
if(NOT LIB)
    message(FATAL_ERROR "nn_prov: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
set(srcs "")
if(CONFIG_NN_FEATURE_BLE AND CONFIG_NN_FEATURE_WIFI)
    set(srcs "${LIB}/src/nn_prov.c" "${LIB}/src/nn_prov_crypto.c"
             "${LIB}/src/nn_prov_ble.c" "${LIB}/src/nn_prov_cli.c")
endif()
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${LIB}/include"
    PRIV_INCLUDE_DIRS "${LIB}/src"
    REQUIRES "nn_registry" "nn_netstream" "nn_pal"
    PRIV_REQUIRES "nvs_flash" "console" "bt" "nn_crypto"
)
