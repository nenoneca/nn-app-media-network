# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_registry into the media build.
# The source is REUSED, not inlined: a standalone app clone vendors it via the
# nn-modules submodule (./nn-modules/modules/libs/nn_registry); the monorepo
# uses device/modules/libs/nn_registry.  First existing path wins.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_registry"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_registry"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_registry")
    if(EXISTS "${_c}")
        set(REG "${_c}")
        break()
    endif()
endforeach()
if(NOT REG)
    message(FATAL_ERROR "nn_registry: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()
idf_component_register(
    SRCS "${REG}/src/nn_registry.c"
    INCLUDE_DIRS "${REG}/include"
)
