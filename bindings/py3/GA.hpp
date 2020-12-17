#ifndef BINDINGS_PY3_GA_HPP
#define BINDINGS_PY3_GA_HPP

#include <pybind11/pybind11.h>

namespace py = pybind11;

void createGABindings(py::module &m);

#endif
