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
# if fftw3f static library is found (optional), this helper provides variable FFTW3F_STATIC_LIBRARIES and CMake target FFTW3::fftw3f-static
# if fftw3l library is found (optional), this helper provides variable FFTW3L_LIBRARIES and CMake target FFTW3::fftw3l
# if fftw3l static library is found (optional), this helper provides variable FFTW3L_STATIC_LIBRARIES and CMake target FFTW3::fftw3l-static
# if fftw3q library is found (optional), this helper provides variable FFTW3Q_LIBRARIES and CMake target FFTW3::fftw3Q
# if fftw3q static library is found (optional), this helper provides variable FFTW3Q_STATIC_LIBRARIES and CMake target FFTW3::fftw3Q-static
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

find_library(FFTW3_STATIC_LIBRARY
    NAMES libfftw3.a
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3F_LIBRARY
    NAMES fftw3f
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3F_STATIC_LIBRARY
    NAMES libfftw3f.a
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3L_LIBRARY
    NAMES fftw3l
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3L_STATIC_LIBRARY
    NAMES libfftw3l.a
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3Q_LIBRARY
    NAMES fftw3q
    HINTS ${FFTW_LIB_SEARCHPATH})

find_library(FFTW3Q_STATIC_LIBRARY
    NAMES libfftw3q.a
    HINTS ${FFTW_LIB_SEARCHPATH})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR)

mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARY)

set(_FFTW3_LIBRARY_TYPE SHARED)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(_FFTW3_LIBRARY_TYPE UNKNOWN)
endif()

if (FFTW3_FOUND)
    set(FFTW3_LIBRARIES ${FFTW3_LIBRARY})
    set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
    if (NOT TARGET FFTW3::fftw3)
        # double precision
        add_library(FFTW3::fftw3 ${_FFTW3_LIBRARY_TYPE} IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3")
    endif()

    if (FFTW3_STATIC_LIBRARY AND NOT(TARGET FFTW3::fftw3-static))
        # double precision
        set(FFTW3_STATIC_LIBRARIES ${FFTW3_STATIC_LIBRARY})
        add_library(FFTW3::fftw3-static STATIC IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3-static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3_STATIC_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3-static")
    endif()

    if (FFTW3F_LIBRARY AND NOT(TARGET FFTW3::fftw3f))
        # float precision
        set(FFTW3F_LIBRARIES ${FFTW3F_LIBRARY})
        add_library(FFTW3::fftw3f ${_FFTW3_LIBRARY_TYPE} IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3f PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3F_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3f")
    endif()

    if (FFTW3F_STATIC_LIBRARY AND NOT(TARGET FFTW3::fftw3f-static))
        # float precision
        set(FFTW3F_STATIC_LIBRARIES ${FFTW3F_STATIC_LIBRARY})
        add_library(FFTW3::fftw3f-static STATIC IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3f-static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3F_STATIC_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3f-static")
    endif()

    if (FFTW3L_LIBRARY AND NOT(TARGET FFTW3::fftw3l))
        # long double precision
        set(FFTW3L_LIBRARIES ${FFTW3F_LIBRARY})
        add_library(FFTW3::fftw3l ${_FFTW3_LIBRARY_TYPE} IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3l PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3L_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3l")
    endif()

    if (FFTW3L_STATIC_LIBRARY AND NOT(TARGET FFTW3::fftw3l-static))
        # long double precision
        set(FFTW3L_STATIC_LIBRARIES ${FFTW3L_STATIC_LIBRARY})
        add_library(FFTW3::fftw3l-static STATIC IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3l-static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3L_STATIC_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3l-static")
    endif()

    if (FFTW3Q_LIBRARY AND NOT(TARGET FFTW3::fftw3q))
        # quad precision
        set(FFTW3Q_LIBRARIES ${FFTW3F_LIBRARY})
        add_library(FFTW3::fftw3q ${_FFTW3_LIBRARY_TYPE} IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3q PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3Q_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3q")
    endif()

    if (FFTW3Q_STATIC_LIBRARY AND NOT(TARGET FFTW3::fftw3q-static))
        # quad precision
        set(FFTW3Q_STATIC_LIBRARIES ${FFTW3Q_STATIC_LIBRARY})
        add_library(FFTW3::fftw3q-static STATIC IMPORTED GLOBAL)
        set_target_properties(FFTW3::fftw3q-static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIRS}
            IMPORTED_LOCATION ${FFTW3Q_STATIC_LIBRARIES})
        list(APPEND FFTW3_TARGETS "FFTW3::fftw3q-static")
    endif()
endif()

string(REPLACE ";" ", " FFTW3_TARGETS_STR "${FFTW3_TARGETS}")
message(STATUS "find_package(FFTW3) provides CMake targets: ${FFTW3_TARGETS_STR}")
