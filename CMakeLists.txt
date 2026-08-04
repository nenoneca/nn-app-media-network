# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.16)

# Shared nn camera-system components.  In the media/ monorepo they live one
# level up (../components); in a standalone clone of this app repo they are
# vendored into ./components.  Prefer the in-repo copy when present.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/components")
    set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")
else()
    set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../components")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(nn-app-media-network)
