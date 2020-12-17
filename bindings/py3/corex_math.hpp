#ifndef BINDINGS_PY3_COREX_MATH_HPP
#define BINDINGS_PY3_COREX_MATH_HPP

#include <pybind11/pybind11.h>

namespace py = pybind11;

void createRequiredCoreXMathBindings(py::module &m);

#endif
