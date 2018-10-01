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

# unit_test_init_data copies tests data into the expected directories
add_custom_target(unit_test_init_data ALL
                      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/input"
                      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/filters"
                      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/output"
                      COMMAND ${CMAKE_COMMAND} -E copy_directory
                          "${CMAKE_SOURCE_DIR}/data/input"
                          "${CMAKE_CURRENT_BINARY_DIR}/input"
                      COMMAND ${CMAKE_COMMAND} -E copy_directory
                          "${CMAKE_SOURCE_DIR}/data/filters"
                          "${CMAKE_CURRENT_BINARY_DIR}/filters")
set_target_properties(unit_test_init_data PROPERTIES FOLDER test)

# unit_test_main contains catch2 main (speed up unit test compilation)
add_library(unit_test_main STATIC EXCLUDE_FROM_ALL catch_main.cc)
set_target_properties(unit_test_main PROPERTIES FOLDER test)
target_link_libraries(unit_test_main catch)

add_library(unit_test_lib STATIC EXCLUDE_FROM_ALL
    utils.h
    utils.cc)
target_include_directories(unit_test_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(unit_test_lib PROPERTIES FOLDER test)
target_link_libraries(unit_test_lib
    libsirius-static
    libsirius-gdal-static
    Threads::Threads)

file(GLOB unit_tests *_tests.cc)

# group test targets into one to make build easier
add_custom_target(build_tests)
set_target_properties(build_tests PROPERTIES FOLDER test)
add_dependencies(build_tests unit_test_init_data)

# create unit test target from test file
# need CMake regeneration to take into account new unit tests
foreach(unit_test_file ${unit_tests})
    string(REGEX REPLACE ".*/(.*).cc" "\\1" unit_test "${unit_test_file}")
    add_executable(${unit_test} EXCLUDE_FROM_ALL ${unit_test_file})
    set_target_properties(${unit_test} PROPERTIES
        FOLDER test
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(${unit_test}
        libsirius-static
        unit_test_lib
        unit_test_main)
    add_test(NAME ${unit_test} COMMAND ${unit_test}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(build_tests ${unit_test})
endforeach()
