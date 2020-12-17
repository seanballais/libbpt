#include <pybind11/pybind11.h>

#include <bpt/bpt.hpp>

#include <GA.hpp>
#include <eastl.hpp>

namespace py = pybind11;

using namespace bpt;

void createGABindings(py::module &m)
{
  py::class_<GA>(m, "GA")
    .def(py::init())
    .def("generateSolutions", &GA::generateSolutions)
    .def("getSolutionFitness", &GA::getSolutionFitness)
    .def("getCurrentRunGenerationNumber", &GA::getCurrentRunGenerationNumber)
    .def("getRecentRunAverageFitnesses", &GA::getRecentRunAverageFitnesses)
    .def("getRecentRunBestFitnesses", &GA::getRecentRunBestFitnesses)
    .def("getRecentRunWorstFitnesses", &GA::getRecentRunWorstFitnesses);
}
