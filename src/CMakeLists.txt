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

set(SIRIUS_SRC
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

    sirius/image_streamer.h
    sirius/image_streamer.cc

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

    # gdal
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
    sirius/gdal/wrapper.cc

    # utils
    sirius/utils/concurrent_queue.h
    sirius/utils/concurrent_queue.txx
    sirius/utils/concurrent_queue_error_code.h
    sirius/utils/concurrent_queue_error_code.cc
    sirius/utils/debug.h
    sirius/utils/debug.cc
    sirius/utils/gsl.h
    sirius/utils/log.h
    sirius/utils/log.cc
    sirius/utils/lru_cache.h
    sirius/utils/numeric.h
    sirius/utils/numeric.cc
    sirius/utils/scope_cleaner.h)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/sirius)

set(SIRIUS_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/sirius/sirius.h.in)
set(SIRIUS_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/include/sirius/sirius.h)

configure_file(${SIRIUS_CONFIG_IN} ${SIRIUS_CONFIG_OUT} @ONLY)

list(APPEND SIRIUS_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include)
LIST(APPEND SIRIUS_LINK_LIBS "gdal" "fftw3" "spdlog" "gsl")

add_library(libsirius SHARED ${SIRIUS_SRC})
set_property(TARGET libsirius PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(libsirius-static STATIC ${SIRIUS_SRC})
set_property(TARGET libsirius-static PROPERTY POSITION_INDEPENDENT_CODE ON)

if (${ENABLE_LOGS})
    # build with logs
    target_compile_definitions(libsirius PUBLIC SIRIUS_ENABLE_LOGS=1)
    target_compile_definitions(libsirius-static PUBLIC SIRIUS_ENABLE_LOGS=1)
endif ()

if (${ENABLE_CACHE_OPTIMIZATION})
    # build with cache
    target_compile_definitions(libsirius PUBLIC SIRIUS_ENABLE_CACHE_OPTIMIZATION=1)
    target_compile_definitions(libsirius-static PUBLIC SIRIUS_ENABLE_CACHE_OPTIMIZATION=1)
endif ()

target_include_directories(libsirius PUBLIC ${SIRIUS_INCLUDE_DIRS})
target_include_directories(libsirius-static PUBLIC ${SIRIUS_INCLUDE_DIRS})

target_link_libraries(libsirius ${SIRIUS_LINK_LIBS})
target_link_libraries(libsirius-static ${SIRIUS_LINK_LIBS})

install(TARGETS libsirius DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS libsirius-static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
# copy sirius include directory into install directory
install(FILES ${SIRIUS_CONFIG_OUT}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sirius)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sirius
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.txx")
# copy spdlog library into install directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog/spdlog/include/spdlog
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)


add_executable(sirius main.cc)
target_link_libraries(sirius libsirius-static cxxopts Threads::Threads)

install(TARGETS sirius DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
