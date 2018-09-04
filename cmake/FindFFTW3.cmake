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

#.rst:
# FindFFTW3
# ---------
#
# Find fftw3 library
#
# Find the native FFTW3 headers and libraries (double precision).
#
# ::
#
#   FFTW3_INCLUDE_DIRS - where to find fftw3.h, etc.
#   FFTW3_LIBRARIES    - List of libraries when using fftw3.
#   FFTW3_FOUND        - True if fftw3 found.
#
# if fftw3 library is found (mandatory), this helper provides CMake target FFTW3::fftw3
# if fftw3f library is found (optional), this helper provides variable FFTW3F_LIBRARIES and CMake target FFTW3::fftw3f
# if fftw3l library is found (optional), this helper provides variable FFTW3L_LIBRARIES and CMake target FFTW3::fftw3l
# if fftw3q library is found (optional), this helper provides variable FFTW3Q_LIBRARIES and CMake target FFTW3::fftw3Q
#
# this implementation is slightly inspired by
#    https://github.com/InsightSoftwareConsortium/ITK/blob/master/CMake/FindFFTW.cmake

find_package(PkgConfig QUIET)
pkg_check_modules(PC_FFTW3 fftw3)

set(FFTW_LIB_SEARCHPATH
    ${PC_FFTW3_INCLUDE_DIRS}
    /usr/lib/fftw
    /usr/local/lib/fftw)

if (${FFTW_INCLUDE_PATH})
    get_filename_component(FFTW_INSTALL_BASE_PATH ${FFTW_INCLUDE_PATH} PATH)
    list(APPEND FFTW_LIB_SEARCHPATH
        ${FFTW_INSTALL_BASE_PATH}/lib
        ${FFTW_INSTALL_BASE_PATH}/lib64
        ${FFTW_INSTALL_BASE_PATH}/bin)
endif()

find_path(FFTW3_INCLUDE_DIR
    NAMES fftw3.h
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3_LIBRARY
    NAMES fftw3
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3F_LIBRARY
    NAMES fftw3f
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3L_LIBRARY
    NAMES fftw3l
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3Q_LIBRARY
    NAMES fftw3q
    HINTS ${FFTW_LIB_SEARCHPATH})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    find_file(FFTW3_DLL_LIBRARY
        NAMES fftw3.dll
        HINTS ${FFTW_LIB_SEARCHPATH})
    find_file(FFTW3F_DLL_LIBRARY
        NAMES fftw3f.dll
        HINTS ${FFTW_LIB_SEARCHPATH})
    find_file(FFTW3L_DLL_LIBRARY
        NAMES fftw3l.dll
        HINTS ${FFTW_LIB_SEARCHPATH})
    find_file(FFTW3Q_DLL_LIBRARY
        NAMES fftw3q.dll
        HINTS ${FFTW_LIB_SEARCHPATH})
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR)

mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARY)

if (FFTW3_FOUND)
    set(FFTW3_LIBRARIES ${FFTW3_LIBRARY})
    set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
    if (NOT TARGET FFTW3::fftw3)
        # double precision
        add_library(FFTW3::fftw3 SHARED IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS})
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set_target_properties(FFTW3::fftw3 PROPERTIES
                IMPORTED_LOCATION ${FFTW3_DLL_LIBRARY}
                IMPORTED_IMPLIB ${FFTW3_LIBRARIES})
        else()
            set_target_properties(FFTW3::fftw3 PROPERTIES
                IMPORTED_LOCATION ${FFTW3_LIBRARIES})
        endif()
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3")
    endif()

    if (FFTW3F_LIBRARY AND NOT(TARGET FFTW3::fftw3f))
        # float precision
        set(FFTW3F_LIBRARIES ${FFTW3F_LIBRARY})
        add_library(FFTW3::fftw3f SHARED IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3f PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS})
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set_target_properties(FFTW3::fftw3f PROPERTIES
                IMPORTED_LOCATION ${FFTW3F_DLL_LIBRARY}
                IMPORTED_IMPLIB ${FFTW3F_LIBRARIES})
        else()
            set_target_properties(FFTW3::fftw3f PROPERTIES
                IMPORTED_LOCATION ${FFTW3F_LIBRARIES})
        endif()
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3f")
    endif()

    if (FFTW3L_LIBRARY AND NOT(TARGET FFTW3::fftw3l))
        # long double precision
        set(FFTW3L_LIBRARIES ${FFTW3F_LIBRARY})
        add_library(FFTW3::fftw3l SHARED IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3l PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS})
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set_target_properties(FFTW3::fftw3l PROPERTIES
                IMPORTED_LOCATION ${FFTW3L_DLL_LIBRARY}
                IMPORTED_IMPLIB ${FFTW3L_LIBRARIES})
        else()
            set_target_properties(FFTW3::fftw3l PROPERTIES
                IMPORTED_LOCATION ${FFTW3L_LIBRARIES})
        endif()
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3l")
    endif()

    if (FFTW3Q_LIBRARY AND NOT(TARGET FFTW3::fftw3q))
        # quad precision
        set(FFTW3Q_LIBRARIES ${FFTW3F_LIBRARY})
        add_library(FFTW3::fftw3q SHARED IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3q PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS})
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set_target_properties(FFTW3::fftw3q PROPERTIES
                IMPORTED_LOCATION ${FFTW3Q_DLL_LIBRARY}
                IMPORTED_IMPLIB ${FFTW3Q_LIBRARIES})
        else()
            set_target_properties(FFTW3::fftw3q PROPERTIES
                IMPORTED_LOCATION ${FFTW3Q_LIBRARIES})
        endif()
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3q")
    endif()
endif()

string(REPLACE ";" ", " FFTW3_TARGETS_STR "${FFTW3_TARGETS}")
message(STATUS "find_package(FFTW3) provides CMake targets: ${FFTW3_TARGETS_STR}")
