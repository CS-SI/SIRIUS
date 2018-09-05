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

# create library from package dependencies
find_package(FFTW3 REQUIRED)

add_subdirectory(spdlog)
add_subdirectory(gsl)

if (ENABLE_SIRIUS_EXECUTABLE OR BUILD_TESTING)
    find_package(GDAL REQUIRED)

    add_library(gdal SHARED IMPORTED GLOBAL)
    set_target_properties(gdal PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GDAL_INCLUDE_DIR}
        INTERFACE_COMPILE_DEFINITIONS CPL_LOG=${ENABLE_LOGS})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set_target_properties(gdal PROPERTIES
            IMPORTED_IMPLIB ${GDAL_LIBRARY})
    else()
        set_target_properties(gdal PROPERTIES
            IMPORTED_LOCATION ${GDAL_LIBRARY})
    endif()
endif ()

if (ENABLE_SIRIUS_EXECUTABLE)
    add_subdirectory(cxxopts)
endif()

if (BUILD_TESTING)
    add_subdirectory(catch)
endif()
