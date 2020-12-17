#include <pybind11/pybind11.h>

#include <bpt/bpt.hpp>

#include <ds.hpp>

namespace py = pybind11;

using namespace bpt;

void createDSBindings(py::module &m)
{
  py::class_<InputBuilding>(m, "InputBuilding")
    .def(py::init())
    .def(py::init([](float length, float width) {
      return new InputBuilding{ length, width };
    }))
    .def_readwrite("length", &InputBuilding::length)
    .def_readwrite("width", &InputBuilding::width);

  py::class_<Solution>(m, "Solution")
    .def(py::init())
    .def(py::init<const Solution&>())
    .def(py::init<int>())
    .def("setBuildingXPos", &Solution::setBuildingXPos)
    .def("setBuildingYPos", &Solution::setBuildingYPos)
    .def("setBuildingRotation", &Solution::setBuildingRotation)
    .def("setFitness", &Solution::setFitness)
    .def("getBuildingXPos", &Solution::getBuildingXPos)
    .def("getBuildingYPos", &Solution::getBuildingYPos)
    .def("getBuildingRotation", &Solution::getBuildingRotation)
    .def("getNumBuildings", &Solution::getNumBuildings)
    .def("getFitness", &Solution::getFitness)
    .def("__eq__", &Solution::operator==, py::is_operator())
    .def("__ne__", &Solution::operator!=, py::is_operator());
}
