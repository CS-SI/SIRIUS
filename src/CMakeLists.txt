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

set(LIBSIRIUS_SRC
    sirius/exception.h

    sirius/types.h
    sirius/types.cc

    sirius/filter.h
    sirius/filter.cc
    sirius/image.h
    sirius/image.cc

    sirius/i_frequency_resampler.h
    sirius/frequency_resampler_factory.h
    sirius/frequency_resampler_factory.cc

    sirius/i_frequency_translator.h
    sirius/frequency_translator_factory.h
    sirius/frequency_translator_factory.cc

    sirius/i_frequency_rotator.h
    sirius/frequency_rotator_factory.h
    sirius/frequency_rotator_factory.cc

    # image_decomposition
    sirius/image_decomposition/periodic_smooth_policy.h
    sirius/image_decomposition/periodic_smooth_policy.txx
    sirius/image_decomposition/regular_policy.h
    sirius/image_decomposition/regular_policy.txx

    # resampling
    sirius/resampling/frequency_resampler.h
    sirius/resampling/frequency_resampler.txx
    sirius/resampling/interpolator.h
    sirius/resampling/interpolator.cc

    # resampling upsampling strategies
    sirius/resampling/upsampling/periodization_upsampling.h
    sirius/resampling/upsampling/periodization_upsampling.cc
    sirius/resampling/upsampling/zero_padding_upsampling.h
    sirius/resampling/upsampling/zero_padding_upsampling.cc

    # translation
    sirius/translation/frequency_translator.h
    sirius/translation/interpolator.h
    sirius/translation/interpolator.cc
    sirius/translation/parameters.h
    sirius/translation/processor.h
    sirius/translation/processor.cc
    sirius/translation/utils.h
    sirius/translation/utils.cc

    # rotation
    sirius/rotation/frequency_rotator.h
    sirius/rotation/interpolator.h
    sirius/rotation/interpolator.cc
    sirius/rotation/parameters.h
    sirius/rotation/processor.cc
    sirius/rotation/processor.h
    sirius/rotation/utils.cc
    sirius/rotation/utils.h

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

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/sirius)

set(SIRIUS_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/sirius/sirius.h.in)
set(SIRIUS_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/include/sirius/sirius.h)

configure_file(${SIRIUS_CONFIG_IN} ${SIRIUS_CONFIG_OUT} @ONLY)

list(APPEND LIBSIRIUS_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include)
LIST(APPEND LIBSIRIUS_LINK_LIBS "fftw3" "spdlog" "gsl")

add_library(libsirius SHARED ${LIBSIRIUS_SRC})
set_property(TARGET libsirius PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(libsirius-static STATIC ${LIBSIRIUS_SRC})
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

target_include_directories(libsirius PUBLIC ${LIBSIRIUS_INCLUDE_DIRS})
target_include_directories(libsirius-static PUBLIC ${LIBSIRIUS_INCLUDE_DIRS})

target_link_libraries(libsirius ${LIBSIRIUS_LINK_LIBS})
target_link_libraries(libsirius-static ${LIBSIRIUS_LINK_LIBS})

install(TARGETS libsirius DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/sirius)
install(TARGETS libsirius-static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/sirius)
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
    set(LIBSIRIUS_GDAL_SRC
        # gdal
        sirius/gdal/error_code.h
        sirius/gdal/error_code.cc
        sirius/gdal/exception.h
        sirius/gdal/exception.cc
        sirius/gdal/image_streamer.h
        sirius/gdal/image_streamer.txx
        sirius/gdal/input_stream.h
        sirius/gdal/input_stream.cc
        sirius/gdal/output_stream.h
        sirius/gdal/output_stream.cc
        sirius/gdal/types.h
        sirius/gdal/wrapper.h
        sirius/gdal/wrapper.cc

        # gdal/resampling
        sirius/gdal/resampling/input_stream.h
        sirius/gdal/resampling/input_stream.cc
        sirius/gdal/resampling/output_stream.h
        sirius/gdal/resampling/output_stream.cc

        # gdal/translation
        sirius/gdal/translation/input_stream.h
        sirius/gdal/translation/input_stream.cc
        sirius/gdal/translation/output_stream.h
        sirius/gdal/translation/output_stream.cc

        # gdal/rotation
        sirius/gdal/rotation/input_stream.h
        sirius/gdal/rotation/input_stream.cc
        sirius/gdal/rotation/output_stream.h
        sirius/gdal/rotation/output_stream.cc)
    add_library(libsirius-gdal-static STATIC EXCLUDE_FROM_ALL ${LIBSIRIUS_GDAL_SRC})
    target_link_libraries(libsirius-gdal-static gdal libsirius)
endif ()

if (ENABLE_SIRIUS_EXECUTABLE)
    add_executable(sirius main.cc)
    target_link_libraries(sirius
        libsirius-static
        libsirius-gdal-static
        cxxopts
        Threads::Threads)

    install(TARGETS sirius DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()