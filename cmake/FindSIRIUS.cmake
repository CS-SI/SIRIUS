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
# FindSIRIUS
# ---------
#
# Find sirius library
#
# Find the native SIRIUS headers and libraries.
# Set SIRIUS_ROOT to give a directory hint
#
# ::
#
#   SIRIUS_INCLUDE_DIRS        - where to find sirius.h, etc.
#   SIRIUS_LIBRARIES           - List of libraries when using sirius.
#   SIRIUS_STATIC_LIBRARIES    - List of static libraries when using sirius.
#   SIRIUS_FOUND               - True if sirius found.

find_path(SIRIUS_INCLUDE_DIR
    sirius/sirius.h
    HINTS
        ${SIRIUS_ROOT}
    PATH_SUFFIXES
        include/sirius)

find_library(SIRIUS_LIBRARY
    NAMES
        sirius # linux/os x naming (default library prefix is lib)
        libsirius # windows naming (default library prefix is '')
    HINTS
        ${SIRIUS_ROOT}
    PATH_SUFFIXES
        lib/sirius)

if (WIN32)
    find_file(SIRIUS_DLL_LIBRARY
        NAMES libsirius.dll
    HINTS
        ${SIRIUS_ROOT}
    PATH_SUFFIXES
        lib/sirius)
endif()

find_library(SIRIUS_STATIC_LIBRARY
    NAMES
        sirius-static # linux/os x naming (default library prefix is lib)
        libsirius-static # windows naming (default library prefix is '')
    HINTS
        ${SIRIUS_ROOT}
    PATH_SUFFIXES
        lib/sirius)

set(SIRIUS_LIBRARIES ${SIRIUS_LIBRARY})
set(SIRIUS_STATIC_LIBRARIES ${SIRIUS_STATIC_LIBRARY})
set(SIRIUS_INCLUDE_DIRS ${SIRIUS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SIRIUS DEFAULT_MSG SIRIUS_LIBRARY SIRIUS_STATIC_LIBRARY SIRIUS_INCLUDE_DIR)

mark_as_advanced(SIRIUS_INCLUDE_DIR SIRIUS_LIBRARY SIRIUS_STATIC_LIBRARY)

if (SIRIUS_FOUND)
    if(NOT(TARGET sirius::shared) AND NOT(TARGET sirius::static))
        add_library(sirius::shared SHARED IMPORTED GLOBAL)
        set_target_properties(sirius::shared PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SIRIUS_INCLUDE_DIRS})
        if (WIN32)
            set_target_properties(sirius::shared PROPERTIES
                IMPORTED_LOCATION ${SIRIUS_DLL_LIBRARY}
                IMPORTED_IMPLIB ${SIRIUS_LIBRARIES})
        else()
            set_target_properties(sirius::shared PROPERTIES
                IMPORTED_LOCATION ${SIRIUS_LIBRARIES})
        endif()

        add_library(sirius::static STATIC IMPORTED GLOBAL)
        set_target_properties(sirius::static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SIRIUS_INCLUDE_DIRS}
            IMPORTED_LOCATION ${SIRIUS_STATIC_LIBRARIES})
        message(STATUS "find_package(SIRIUS) provides CMake targets: sirius::shared and sirius::static")
    endif()
endif ()

