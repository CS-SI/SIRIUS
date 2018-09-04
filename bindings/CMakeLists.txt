set(SIRIUS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src")
set(SIRIUS_LIBRARY_DIR "${PROJECT_BINARY_DIR}/src")
set(SPDLOG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third_party/spdlog/spdlog/include")
find_path(PYBIND11_INCLUDE_DIR pybind11/pybind11.h)
if(NOT PYBIND11_INCLUDE_DIR)
  message(FATAL_ERROR "Cannot find PYBIND11_INCLUDE_DIR")
endif()

find_package ( PythonInterp 3.0 REQUIRED)

configure_file(setup.cfg.in ${CMAKE_CURRENT_SOURCE_DIR}/setup.cfg )

set(stamp_file "${CMAKE_CURRENT_BINARY_DIR}/stamp.build")
add_custom_command(OUTPUT "${stamp_file}"
  COMMAND ${PYTHON_EXECUTABLE} setup.py build --build-lib ${CMAKE_CURRENT_BINARY_DIR} --define SIRIUS_ENABLE_LOGS=${SIRIUS_ENABLE_LOGS} --define SIRIUS_ENABLE_CACHE_OPTIMIZATION=${SIRIUS_ENABLE_CACHE_OPTIMIZATION}
  COMMAND ${CMAKE_COMMAND} -E touch "${stamp_file}"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "${PYTHON_EXECUTABLE} setup.py build"
  VERBATIM)

add_custom_target(siriuspy ALL DEPENDS libsirius ${stamp_file})

add_custom_target(test_siriuspy COMMAND PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR} ${PYTHON_EXECUTABLE} -m unittest test.py  -v)

#python3 setup.py build_ext --include-dirs=SIRIUS_INCLUDE_DIR:PYBIND11_INCLUDE_DIR:FFTW_INCLUDE_DIR --library-dirs=SIRIUS_LIB_DIR

