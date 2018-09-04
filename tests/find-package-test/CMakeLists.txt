#
# Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
#
# This file is part of Sirius
#
#     https://github.com/CS-SI/SIRIUS
#
# Sirius is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Sirius is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sirius.  If not, see <https://www.gnu.org/licenses/>.
#

# This project aims to test find_package(SIRIUS) helper:
# * find SIRIUS shared and static libraries from Sirius install directory (set SIRIUS_ROOT variable)
# * generate small test executables and link with Sirius static and shared libraries

cmake_minimum_required(VERSION 3.2)

if(POLICY CMP0074)
    # find_package() uses <PackageName>_ROOT variables: https://cmake.org/cmake/help/git-stage/policy/CMP0074.html
    cmake_policy(SET CMP0074 NEW)
endif()

project(TestFindPackage CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# path to additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/BuildType.cmake)

# FFTW3 headers are required
find_package(FFTW3 REQUIRED)

# set CXX flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # GCC or Clang
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    if (ENABLE_CLANG_LIBCPP)
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

find_package(SIRIUS REQUIRED)

add_executable(basic_sirius_shared_test
    basic_sirius_test.cc)
target_link_libraries(basic_sirius_shared_test SIRIUS::libsirius FFTW3::fftw3)

add_executable(basic_sirius_static_test
    basic_sirius_test.cc)
target_link_libraries(basic_sirius_static_test SIRIUS::libsirius-static FFTW3::fftw3)