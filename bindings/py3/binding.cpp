#include <pybind11/pybind11.h>

#include <corex_math.hpp>
#include <ds.hpp>
#include <enums.hpp>
#include <GA.hpp>

namespace py = pybind11;

PYBIND11_MODULE(pylibbpt, m)
{
  createDSBindings(m);
  createEnumBindings(m);
  createGABindings(m);

  // Minimal CoreX functions and data structures libbpt
  // users need.
  py::module cx_submodule = m.def_submodule("cx");
  createRequiredCoreXMathBindings(cx_submodule);
}
