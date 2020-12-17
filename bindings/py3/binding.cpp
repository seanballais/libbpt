#include <pybind11/pybind11.h>

#include <ds.hpp>
#include <enums.hpp>

namespace py = pybind11;

PYBIND11_MODULE(pylibbpt, m)
{
  createDSBindings(m);
  createEnumBindings(m);
}
