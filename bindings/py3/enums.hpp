#ifndef BINDINGS_PY3_ENUMS_HPP
#define BINDINGS_PY3_ENUMS_HPP

#include <pybind11/pybind11.h>

namespace py = pybind11;

void createEnumBindings(py::module &m);

#endif
