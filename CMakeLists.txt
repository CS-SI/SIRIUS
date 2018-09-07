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

cmake_minimum_required(VERSION 3.2)

# path to additional cmake modules
list(APPEND CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
     "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmake-modules")

set(SIRIUS_VERSION "" CACHE STRING "Sirius version")
set(SIRIUS_REVISION_COMMIT "" CACHE STRING "Sirius revision commit")

include(cmake/BuildType.cmake)
include(cmake/ExtractVersion.cmake)

project(SIRIUS VERSION ${SIRIUS_VERSION} LANGUAGES CXX)

option(ENABLE_SIRIUS_EXECUTABLE "Enable Sirius executable target" ON)
option(ENABLE_CACHE_OPTIMIZATION "Enable cache optimization (FFTW plan, Filter FFT)" ON)
option(ENABLE_LOGS "Enable logs" ON)
option(ENABLE_GSL_CONTRACTS "Enable GSL contracts" OFF)
option(ENABLE_DOCUMENTATION "Enable documentation generation" OFF)
option(BUILD_TESTING "Enable unit tests" OFF)
option(USE_CXX_STATIC_RUNTIME "Build targets with C++ static runtime library" OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    option(ENABLE_CLANG_LIBCPP "Force use of libc++" OFF)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "Force GSL contracts on Debug build type")
    set(ENABLE_GSL_CONTRACTS "ON")
endif()

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "Sirius ${SIRIUS_EXTENDED_VERSION} (${SIRIUS_REVISION_COMMIT})")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Enable Sirius executable: ${ENABLE_SIRIUS_EXECUTABLE}")
message(STATUS "Enable cache: ${ENABLE_CACHE_OPTIMIZATION}")
message(STATUS "Enable logs: ${ENABLE_LOGS}")
message(STATUS "Enable GSL contracts: ${ENABLE_GSL_CONTRACTS}")
message(STATUS "Enable documentation: ${ENABLE_DOCUMENTATION}")
message(STATUS "Enable unit tests: ${BUILD_TESTING}")
message(STATUS "Link Sirius binaries with static C++ runtime library: ${USE_CXX_STATIC_RUNTIME}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(STATUS "Force use of libc++: ${ENABLE_CLANG_LIBCPP}")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set CXX global flags
set(SIRIUS_CXX_FLAGS "")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # GCC or Clang
    set(SIRIUS_CXX_FLAGS  "${SIRIUS_CXX_FLAGS} -Wall -Wextra")
    if (ENABLE_CLANG_LIBCPP)
        set(SIRIUS_CXX_FLAGS  "${SIRIUS_CXX_FLAGS} -stdlib=libc++")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # MSVC compiler
    # /D_USE_MATH_DEFINES will allow M_PI declaration from #include <cmath>
    set(SIRIUS_CXX_FLAGS "${SIRIUS_CXX_FLAGS} /D_USE_MATH_DEFINES")
    # disable 4251 (from gdal): 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
    set(SIRIUS_CXX_FLAGS "${SIRIUS_CXX_FLAGS} /wd4251")
endif()

if (USE_CXX_STATIC_RUNTIME)
    # build with C++ static runtime flags
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # extracted from https://gitlab.kitware.com/cmake/community/wikis/FAQ#dynamic-replace
        foreach(flag_var
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
        endforeach(flag_var)
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SIRIUS_CXX_FLAGS}")

# define GNU standard installation directories (CMAKE_INSTALL_XXXDIR)
include(GNUInstallDirs)

# define common variables for install directory structure
set(SIRIUS_CONFIG_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/cmake")
set(SIRIUS_CONFIG_CMAKE_MODULES_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/cmake-modules")
set(SIRIUS_CONFIG_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/sirius")
set(SIRIUS_CONFIG_TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(SIRIUS_CONFIG_NAMESPACE "${PROJECT_NAME}::")

find_package(Threads)

add_subdirectory(third_party)
add_subdirectory(src)

# copy find_package as module for SIRIUS and FFTW3
install(FILES cmake/FindSIRIUS.cmake DESTINATION ${SIRIUS_CONFIG_CMAKE_MODULES_INSTALL_DIR})
install(FILES cmake/FindFFTW3.cmake DESTINATION ${SIRIUS_CONFIG_CMAKE_MODULES_INSTALL_DIR})

# add namespace "SIRIUS::" to exported targets
install(
    EXPORT "${SIRIUS_CONFIG_TARGETS_EXPORT_NAME}"
    NAMESPACE "${SIRIUS_CONFIG_NAMESPACE}"
    DESTINATION "${SIRIUS_CONFIG_CMAKE_INSTALL_DIR}"
)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests/input
                        ${CMAKE_CURRENT_BINARY_DIR}/tests/filters
                        ${CMAKE_CURRENT_BINARY_DIR}/tests/output)
    file(COPY data/input DESTINATION tests)
    file(COPY data/filters DESTINATION tests)
endif()

if (ENABLE_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYFILE_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/doxyfile.in)
        set(DOXYFILE_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen/doxyfile)

        configure_file(${DOXYFILE_CONFIG_IN} ${DOXYFILE_CONFIG_OUT} @ONLY)

        add_custom_target(doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_CONFIG_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
        # find_package(LATEX QUIET)
        # if (LATEX_FOUND)
        #     add_custom_command(TARGET doc
        #         POST_BUILD
        #         COMMAND "${CMAKE_MAKE_PROGRAM}"
        #         COMMENT	"Generating documentation pdf"
        #         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/latex")
        #     # copy pdf manual into install directory
        #     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        #             DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/api-doc
        #             RENAME manual.pdf)
        # else()
        #     message(STATUS "Cannot build pdf manual (LaTeX not found)")
        # endif()
    # copy doxygen documentation into install directory
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
            DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/api-doc)
    else()
        message(STATUS "Doxygen is needed to build the documentation.")
    endif()
endif()
