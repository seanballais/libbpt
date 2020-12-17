cmake_minimum_required(VERSION 3.14)

add_library(libbpt
    GA.cpp
    ds/Solution.cpp
    # So that CLion and IDEs that have CMake integration will know that the
    # header-only files are part of the project.
    bpt.hpp
    ds.hpp
    SelectionType.hpp
    ds/InputBuilding.hpp
)
