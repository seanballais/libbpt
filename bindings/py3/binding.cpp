#include <pybind11/pybind11.h>

#include <ds.hpp>
#include <enums.hpp>
#include <GA.hpp>

namespace py = pybind11;

PYBIND11_MODULE(pylibbpt, m)
{
  createDSBindings(m);
  createEnumBindings(m);
  createGABindings(m);
}
