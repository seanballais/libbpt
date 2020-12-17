#include <pybind11/pybind11.h>

#include <bpt/bpt.hpp>

#include <enums.hpp>

namespace py = pybind11;

using namespace bpt;

void createEnumBindings(py::module &m)
{
  py::enum_<SelectionType>(m, "SelectionType")
    .value("NONE", SelectionType::NONE)
    .value("RWS", SelectionType::RWS)
    .value("TS", SelectionType::TS);
}
