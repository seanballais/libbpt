#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include <corex/math.hpp>

#include <eastl.hpp>

namespace py = pybind11;

void createRequiredCoreXMathBindings(py::module &m)
{
  py::class_<cx::Vec2>(m, "Vec2")
    .def(py::init())
    .def(py::init<float, float>())
    .def(py::init<const cx::Vec2&>())
    .def_readwrite("x", &cx::Vec2::x)
    .def_readwrite("y", &cx::Vec2::y)
    .def(py::self + py::self)
    .def(py::self - py::self)
    .def(py::self * int())
    .def(py::self * float())
    .def(int() * py::self)
    .def(float() * py::self)
    .def(py::self / int())
    .def(py::self / float());

  py::class_<cx::NPolygon>(m, "NPolygon")
    .def(py::init())
    .def(py::init([](const eastl::vector<cx::Point>& vertices) {
      return new cx::NPolygon{ vertices };
    }))
    .def_readwrite("vertices", &cx::NPolygon::vertices);

  m.attr("Point") = m.attr("Vec2");
}
