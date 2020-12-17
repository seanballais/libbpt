cmake_minimum_required(VERSION 3.14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

pybind11_add_module(pylibbpt
   binding.cpp
   corex_math.cpp
   ds.cpp
   enums.cpp
   GA.cpp
   # So that CLion and IDEs that have CMake integration will know that the
   # header-only files are part of the project.
   eastl.hpp
)
