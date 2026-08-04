# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper for the converged nn_crypto.  Source REUSED, not inlined:
# standalone app clone -> nn-modules submodule; monorepo -> device/modules/libs.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_crypto"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_crypto"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_crypto")
    if(EXISTS "${_c}")
        set(NC "${_c}")
        break()
    endif()
endforeach()
if(NOT NC)
    message(FATAL_ERROR "nn_crypto: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
idf_component_register(
    SRCS "${NC}/src/nn_crypto.c"
    INCLUDE_DIRS "${NC}/include"
    REQUIRES nn_osal mbedtls
)
