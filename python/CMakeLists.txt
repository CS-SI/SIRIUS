# PYBIND11_INCLUDE_DIR is used in pybind11_add_module
set(PYBIND11_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/pybind11/pybind11/include)

include(pybind11Tools)
pybind11_add_module(siriuspy siriuspy.cc)
target_link_libraries(siriuspy PRIVATE libsirius)

if(BUILD_TESTING)
  add_test(NAME py_tests
    COMMAND ${CMAKE_COMMAND} -E env
    PYTHONPATH=${CMAKE_BINARY_DIR}/lib
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_resampler.py)
endif()
