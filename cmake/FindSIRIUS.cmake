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
#   SIRIUS_INCLUDE_DIRS - where to find sirius.h, etc.
#   SIRIUS_LIBRARIES    - List of libraries when using sirius.
#   SIRIUS_FOUND        - True if sirius found.

find_package(PkgConfig)

find_path(SIRIUS_INCLUDE_DIR
    sirius/sirius.h
    HINTS
        ${SIRIUS_ROOT}
        ${PC_SIRIUS_INCLUDE_DIRS}
    PATH_SUFFIXES
        include/sirius)

find_library(SIRIUS_LIBRARY
    NAMES
        libsirius
    HINTS
        ${SIRIUS_ROOT}
        ${PC_SIRIUS_LIBRARIES}
    PATH_SUFFIXES
        lib/sirius)

set(SIRIUS_LIBRARIES ${SIRIUS_LIBRARY})
set(SIRIUS_INCLUDE_DIRS ${SIRIUS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SIRIUS DEFAULT_MSG SIRIUS_LIBRARY SIRIUS_INCLUDE_DIR)

mark_as_advanced(SIRIUS_INCLUDE_DIR SIRIUS_LIBRARY)

if (SIRIUS_FOUND)
    if(NOT TARGET sirius)
        add_library(sirius SHARED IMPORTED)
        set_target_properties(sirius PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${SIRIUS_INCLUDE_DIRS}
            IMPORTED_LOCATION ${SIRIUS_LIBRARIES})
    endif()
endif ()

