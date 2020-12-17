#ifndef BINDINGS_PY3_DS_HPP
#define BINDINGS_PY3_DS_HPP

#include <pybind11/pybind11.h>

namespace py = pybind11;

void createDSBindings(py::module &m);

#endif
