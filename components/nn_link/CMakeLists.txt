# SPDX-License-Identifier: Apache-2.0
# ESP-IDF wrapper bringing the converged nn_link into the media build.  Source
# is REUSED from nn-modules (submodule in a standalone app clone, else the
# monorepo device/modules/libs path).  First existing path wins.
#
# nn_link compiles exactly one backend = {role} x {transport}:
#   role      from IDF_TARGET   (esp32p4 = master, esp32c6 = slave)
#   transport from CONFIG_NN_LINK_TRANSPORT_{SPI,SDIO}
# REQUIRES are resolved at early expansion (CONFIG_* empty there), so they are
# keyed on IDF_TARGET only and list the union of both transports' deps.  SRCS
# are read in the build pass where CONFIG_* is valid, so they pick the transport.
foreach(_c "$ENV{NN_MODULES_DIR}/libs/nn_link"
           "${CMAKE_CURRENT_LIST_DIR}/../../nn-modules/modules/libs/nn_link"
           "${CMAKE_CURRENT_LIST_DIR}/../../../device/modules/libs/nn_link")
    if(EXISTS "${_c}")
        set(LINK "${_c}")
        break()
    endif()
endforeach()
if(NOT LINK)
    message(FATAL_ERROR "nn_link: source not found — init the nn-modules "
            "submodule (git submodule update --init) or set NN_MODULES_DIR")
endif()

set(srcs "${LINK}/src/nn_link_common.c" "${LINK}/src/nn_link_cli.c")
set(reqs "esp_driver_gpio" "console" "nn_registry" "nn_osal")

if(IDF_TARGET STREQUAL "esp32p4")          # master
    list(APPEND reqs "esp_driver_sdmmc" "sdmmc" "esp_driver_spi" "esp_serial_slave_link")
    if(CONFIG_NN_LINK_TRANSPORT_SPI)
        list(APPEND srcs "${LINK}/src/nn_link_spi_master.c")
    else()
        list(APPEND srcs "${LINK}/src/nn_link_sdio_master.c")
    endif()
elseif(IDF_TARGET STREQUAL "esp32c6")      # slave
    list(APPEND reqs "esp_driver_sdio" "esp_driver_spi")
    if(CONFIG_NN_LINK_TRANSPORT_SPI)
        list(APPEND srcs "${LINK}/src/nn_link_spi_slave.c")
    else()
        list(APPEND srcs "${LINK}/src/nn_link_sdio_slave.c")
    endif()
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "${LINK}/include"
    PRIV_INCLUDE_DIRS "${LINK}/src"
    REQUIRES ${reqs}
)
