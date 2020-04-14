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

find_package(FFTW3 REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/sirius)

message(STATUS "Generating sirius/sirius.h")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sirius/sirius.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/sirius/sirius.h
               @ONLY)

# configuration definitions that will populate sirius/config.h
if (ENABLE_LOGS)
    set(SIRIUS_ENABLE_LOGS 1)
endif()
if (ENABLE_CACHE_OPTIMIZATION)
    set(SIRIUS_ENABLE_CACHE_OPTIMIZATION 1)
endif()
if (ENABLE_GSL_CONTRACTS)
    message(STATUS "Enable GSL contract violation policy: ON (terminate on violation)")
    set(GSL_TERMINATE_ON_CONTRACT_VIOLATION 1)
else()
    message(STATUS "Enable GSL contract violation policy: OFF")
    set(GSL_UNENFORCED_ON_CONTRACT_VIOLATION 1)
endif()

message(STATUS "Generating sirius/config.h")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sirius/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/sirius/config.h
               @ONLY)

# sirius shared library (use as a model for compilation options, definitions and link libraries properties of libsirius-objects and libsirius-static)
# TODO: when using cmake 3.12, add all those settings into libsirius-objects only and link libsirius and libsirius-static with libsirius-objects (https://cmake.org/cmake/help/v3.12/release/3.12.html#index-0-command:target_link_libraries)
add_library(libsirius SHARED $<TARGET_OBJECTS:libsirius-objects>)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # export symbols for windows
    set_target_properties(libsirius PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_include_directories(libsirius PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${SIRIUS_CONFIG_INCLUDE_DIR}>)
set_target_properties(libsirius PROPERTIES
    PREFIX ""
    FOLDER "lib")
target_link_libraries(libsirius PUBLIC FFTW3::fftw3 spdlog gsl)

# sirius static library
add_library(libsirius-static STATIC $<TARGET_OBJECTS:libsirius-objects>)
set_target_properties(libsirius-static PROPERTIES
    PREFIX ""
    FOLDER "lib")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set_target_properties(libsirius-static PROPERTIES OUTPUT_NAME "libsirius_s")
else()
    set_target_properties(libsirius-static PROPERTIES OUTPUT_NAME "libsirius")
endif()
target_include_directories(libsirius-static PUBLIC
    $<TARGET_PROPERTY:libsirius,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(libsirius-static PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_DEFINITIONS>)
target_compile_options(libsirius-static PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_OPTIONS>)
target_link_libraries(libsirius-static $<TARGET_PROPERTY:libsirius,INTERFACE_LINK_LIBRARIES>)

# sirius objects library
add_library(libsirius-objects OBJECT EXCLUDE_FROM_ALL
    sirius/sirius.h
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
    sirius/translation/frequency_translator.txx
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
set_target_properties(libsirius-objects PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER lib)
target_include_directories(libsirius-objects PUBLIC
    $<TARGET_PROPERTY:libsirius,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(libsirius-objects PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_DEFINITIONS>)
target_compile_options(libsirius-objects PUBLIC $<TARGET_PROPERTY:libsirius,INTERFACE_COMPILE_OPTIONS>)

# generate find package config mode
include(CMakePackageConfigHelpers)

set(SIRIUS_CMAKE_GENERATED_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/cmake-generated-config")
set(SIRIUS_CONFIG_VERSION "${SIRIUS_CMAKE_GENERATED_CONFIG}/${PROJECT_NAME}ConfigVersion.cmake")
set(SIRIUS_CONFIG_PROJECT "${SIRIUS_CMAKE_GENERATED_CONFIG}/${PROJECT_NAME}Config.cmake")

# generate SIRIUSConfigVersion.cmake file
write_basic_package_version_file(
    "${SIRIUS_CONFIG_VERSION}" COMPATIBILITY SameMajorVersion
)

# generate SIRIUSConfig.cmake file
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${SIRIUS_CONFIG_PROJECT}"
    INSTALL_DESTINATION "${SIRIUS_CONFIG_CMAKE_INSTALL_DIR}"
)

# copy SIRIUSConfig.cmake and SIRIUSConfigVersion.cmake into install directory
install(
    FILES
        "${SIRIUS_CONFIG_PROJECT}"
        "${SIRIUS_CONFIG_VERSION}"
    DESTINATION "${SIRIUS_CONFIG_CMAKE_INSTALL_DIR}"
)

if (NOT DISABLE_INSTALL_HEADERS)
    # copy sirius include directory into install directory
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/sirius
            DESTINATION ${SIRIUS_CONFIG_INCLUDE_DIR}
            FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.txx")
    install(DIRECTORY
                ${CMAKE_CURRENT_SOURCE_DIR}/sirius
            DESTINATION
                ${SIRIUS_CONFIG_INCLUDE_DIR}
            FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.txx"
                PATTERN "gdal*" EXCLUDE)
endif()

# install and export libsirius and libsirius-static
install(TARGETS libsirius libsirius-static
    EXPORT ${SIRIUS_CONFIG_TARGETS_EXPORT_NAME}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${SIRIUS_CONFIG_INCLUDE_DIR}")

if (ENABLE_SIRIUS_EXECUTABLE OR BUILD_TESTING)
    find_package(GDAL REQUIRED)

    add_library(gdal SHARED IMPORTED)
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

    add_library(libsirius-gdal-static STATIC EXCLUDE_FROM_ALL
        # gdal
        sirius/gdal/debug.h
        sirius/gdal/debug.cc

        sirius/gdal/error_code.h
        sirius/gdal/error_code.cc
        sirius/gdal/exception.h
        sirius/gdal/exception.cc
        sirius/gdal/image_streamer.h
        sirius/gdal/image_streamer.txx
        sirius/gdal/i_input_stream.h
        sirius/gdal/i_output_stream.h
        sirius/gdal/input_stream.h
        sirius/gdal/input_stream.cc
        sirius/gdal/output_stream.h
        sirius/gdal/output_stream.cc
        sirius/gdal/stream_block.h
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

    set_target_properties(libsirius-gdal-static PROPERTIES FOLDER lib)
    target_link_libraries(libsirius-gdal-static PUBLIC gdal libsirius-static)
endif ()

if (ENABLE_SIRIUS_EXECUTABLE)
    add_executable(sirius main.cc)
    set_target_properties(sirius PROPERTIES
        FOLDER bin
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(sirius
        libsirius-gdal-static
        cxxopts
        Threads::Threads)

    install(TARGETS sirius
        EXPORT ${SIRIUS_CONFIG_TARGETS_EXPORT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
