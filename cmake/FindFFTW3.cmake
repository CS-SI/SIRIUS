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
# Find fftw3
#
# Find the native FFTW3 headers and libraries (double precision).
#
# ::
#
#   FFTW3_INCLUDE_DIRS - where to find fftw3.h, etc.
#   FFTW3_LIBRARIES    - List of libraries when using fftw3.
#   FFTW3_FOUND        - True if fftw3 found.

# this implementation is inspired by
#    https://github.com/InsightSoftwareConsortium/ITK/blob/master/CMake/FindFFTW.cmake

find_package(PkgConfig)

pkg_check_modules(PC_FFTW3 fftw3)

set(FFTW_LIB_SEARCHPATH
    ${PC_FFTW3_INCLUDE_DIRS}
    /usr/lib/fftw
    /usr/local/lib/fftw)

if (${FFTW_INCLUDE_PATH})
    get_filename_component(FFTW_INSTALL_BASE_PATH ${FFTW_INCLUDE_PATH} PATH)
    list(APPEND FFTW_LIB_SEARCHPATH
        ${FFTW_INSTALL_BASE_PATH}/lib
        ${FFTW_INSTALL_BASE_PATH}/lib64)
endif()

find_path(FFTW3_INCLUDE_DIR fftw3.h HINTS ${FFTW_LIB_SEARCHPATH})
find_library(FFTW3_LIBRARY NAMES fftw3 HINTS ${FFTW_LIB_SEARCHPATH} )

set(FFTW3_LIBRARIES ${FFTW3_LIBRARY} )
set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR )

mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARY )

