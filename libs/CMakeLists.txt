cmake_minimum_required(VERSION 3.14)

# A big project that depends on the same dependencies as libbpt does or a
# sibling dependency may have already added the dependencies we need. If so, we
# no longer need to add the dependencies. Doing so will result in an error. If
# it was not the case, let's add them since we need them. Other projects might
# also need them if libbpt is being used as part of a bigger project.
if (NOT TARGET corex-math)
    add_subdirectory(corex-math/)
endif()

if (NOT TARGET corex-utils)
    add_subdirectory(corex-utils/)
endif()

if (NOT TARGET pybind11_headers)
    add_subdirectory(pybind11/)
endif()
