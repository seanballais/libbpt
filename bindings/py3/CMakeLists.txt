cmake_minimum_required(VERSION 3.14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

pybind11_add_module(pylibbpt
   binding.cpp
   ds.cpp
   enums.cpp
)
