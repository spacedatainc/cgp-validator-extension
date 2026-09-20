# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.23.1)

project(usd-plugins CXX)

# include our packman dependencies
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

if (PXR_VERSION LESS_EQUAL 2311)
    set(CMAKE_CXX_STANDARD 14)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

if (NV_USD)
    # if NV_USD is on, we need to compile with the old C++ ABI
    add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
endif()

# set target directories
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/install/spacedata.cgp_validator_extension")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")

# Set up Omniverse extension build environments.
set(USD_ROOT "${CMAKE_CURRENT_LIST_DIR}/_build/usd-deps/usd")
set(PYTHON_ROOT "${CMAKE_CURRENT_LIST_DIR}/_build/usd-deps/python")
set(Python3_ROOT_DIR "${PYTHON_ROOT}")
set(Python3_EXECUTABLE "${PYTHON_ROOT}/bin/python3")
set(Python3_INCLUDE_DIR "${PYTHON_ROOT}/include/python3.12")
set(Python3_LIBRARY "${PYTHON_ROOT}/lib/libpython3.12.so")
set(Python3_LIBRARY_RELEASE "${Python3_LIBRARY}")
set(Python3_LIBRARY_DEBUG "${Python3_LIBRARY}")
add_subdirectory(src/usd-plugins/cgp-validators)

install(DIRECTORY "src/kit-extension/exts/spacedata.cgp_validator_extension/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
