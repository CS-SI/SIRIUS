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

add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include>
    $<INSTALL_INTERFACE:${SIRIUS_CONFIG_INCLUDE_DIR}>)

# copy spdlog library into install directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include/spdlog
        DESTINATION ${SIRIUS_CONFIG_INCLUDE_DIR})

install(TARGETS spdlog
    EXPORT "${SIRIUS_CONFIG_TARGETS_EXPORT_NAME}"
    INCLUDES DESTINATION "${SIRIUS_CONFIG_INCLUDE_DIR}")

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/LICENSE
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/third_party/spdlog")