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

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/sirius)

set(SIRIUS_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/sirius/sirius.h.in)
set(SIRIUS_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/include/sirius/sirius.h)

configure_file(${SIRIUS_CONFIG_IN} ${SIRIUS_CONFIG_OUT} @ONLY)

# sirius shared library (use as a model for compilation options, definitions and link libraries properties of libsirius-objects and libsirius-static)
# TODO: when using cmake 3.12, add all those settings into libsirius-objects only and link libsirius and libsirius-static with libsirius-objects (https://cmake.org/cmake/help/v3.12/release/3.12.html#index-0-command:target_link_libraries)
add_library(libsirius SHARED $<TARGET_OBJECTS:libsirius-objects>)
if (${ENABLE_LOGS})
    # build with logs
    target_compile_definitions(libsirius PUBLIC SIRIUS_ENABLE_LOGS=1)
endif ()
if (${ENABLE_CACHE_OPTIMIZATION})
    # build with cache
    target_compile_definitions(libsirius PUBLIC SIRIUS_ENABLE_CACHE_OPTIMIZATION=1)
endif ()

target_include_directories(libsirius PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include)
set_target_properties(libsirius PROPERTIES PREFIX "")
target_link_libraries(libsirius PUBLIC fftw3 spdlog gsl)
install(TARGETS libsirius DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/sirius)

# sirius static library
add_library(libsirius-static STATIC $<TARGET_OBJECTS:libsirius-objects>)
set_target_properties(libsirius-static PROPERTIES PREFIX "")
target_include_directories(libsirius-static PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(libsirius-static PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_DEFINITIONS>)
target_compile_options(libsirius-static PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_OPTIONS>)
target_link_libraries(libsirius-static $<TARGET_PROPERTY:libsirius,INTERFACE_LINK_LIBRARIES>)
install(TARGETS libsirius-static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/sirius)

# sirius objects library
add_library(libsirius-objects OBJECT EXCLUDE_FROM_ALL)
set_target_properties(libsirius-objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(libsirius-objects PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(libsirius-objects PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_DEFINITIONS>)
target_compile_options(libsirius-objects PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_OPTIONS>)

target_sources(libsirius-objects PRIVATE
    sirius/sirius.h
    sirius/exception.h

    sirius/types.h
    sirius/types.cc

    sirius/image.h
    sirius/image.cc
    sirius/filter.h
    sirius/filter.cc

    sirius/i_frequency_resampler.h
    sirius/frequency_resampler_factory.h
    sirius/frequency_resampler_factory.cc

    # resampler
    sirius/resampler/frequency_resampler.h
    sirius/resampler/frequency_resampler.txx

    # resampler zoom strategies
    sirius/resampler/zoom_strategy/periodization_strategy.h
    sirius/resampler/zoom_strategy/periodization_strategy.cc
    sirius/resampler/zoom_strategy/zero_padding_strategy.h
    sirius/resampler/zoom_strategy/zero_padding_strategy.cc

    # resampler image decomposition policies
    sirius/resampler/image_decomposition/regular_policy.h
    sirius/resampler/image_decomposition/regular_policy.txx
    sirius/resampler/image_decomposition/periodic_smooth_policy.h
    sirius/resampler/image_decomposition/periodic_smooth_policy.txx

    # fftw
    sirius/fftw/exception.h
    sirius/fftw/exception.cc
    sirius/fftw/fftw.h
    sirius/fftw/fftw.cc
    sirius/fftw/types.h
    sirius/fftw/wrapper.h
    sirius/fftw/wrapper.cc

    # utils
    sirius/utils/concurrent_queue.h
    sirius/utils/concurrent_queue.txx
    sirius/utils/concurrent_queue_error_code.h
    sirius/utils/concurrent_queue_error_code.cc
    sirius/utils/gsl.h
    sirius/utils/log.h
    sirius/utils/log.cc
    sirius/utils/lru_cache.h
    sirius/utils/numeric.h
    sirius/utils/numeric.cc)

# copy sirius include directory into install directory
install(FILES ${SIRIUS_CONFIG_OUT}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sirius/sirius)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sirius
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sirius
        FILES_MATCHING PATTERN "*.h" PATTERN "*.txx")

# copy spdlog library into install directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog/spdlog/include/spdlog
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sirius)

# copy gsl library into install directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/gsl/gsl/include/gsl
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sirius)

if (ENABLE_SIRIUS_EXECUTABLE OR ENABLE_UNIT_TESTS)
    add_library(libsirius-gdal-static STATIC EXCLUDE_FROM_ALL
        # gdal
        sirius/gdal/debug.h
        sirius/gdal/debug.cc
        sirius/gdal/error_code.h
        sirius/gdal/error_code.cc
        sirius/gdal/exception.h
        sirius/gdal/exception.cc
        sirius/gdal/stream_block.h
        sirius/gdal/input_stream.h
        sirius/gdal/input_stream.cc
        sirius/gdal/resampled_output_stream.h
        sirius/gdal/resampled_output_stream.cc
        sirius/gdal/types.h
        sirius/gdal/wrapper.h
        sirius/gdal/wrapper.cc)
    target_link_libraries(libsirius-gdal-static PUBLIC gdal libsirius-static)
endif ()

if (ENABLE_SIRIUS_EXECUTABLE)
    add_executable(sirius
        # main
        sirius/image_streamer.h
        sirius/image_streamer.cc

        main.cc)

    target_link_libraries(sirius
        libsirius-gdal-static
        cxxopts
        Threads::Threads)

    install(TARGETS sirius DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()