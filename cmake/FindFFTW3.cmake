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

find_package(PkgConfig)

pkg_check_modules(PC_FFTW3 fftw3)
find_path(FFTW3_INCLUDE_DIR fftw3.h HINTS ${PC_FFTW3_INCLUDE_DIRS})

find_library(FFTW3_LIBRARY NAMES fftw3 HINTS ${PC_FFTW3_LIBRARY_DIRS} )

set(FFTW3_LIBRARIES ${FFTW3_LIBRARY} )
set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_LIBRARY FFTW3_INCLUDE_DIR )

mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARY )

