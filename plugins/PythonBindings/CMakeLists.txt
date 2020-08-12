set(PYBIND11_CPP_STANDARD -std=c++14)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/pybind11)
pybind11_add_module(test test.cc)
