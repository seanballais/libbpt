#ifndef BINDINGS_PY3_EASTL_HPP
#define BINDINGS_PY3_EASTL_HPP

#include <EASTL/vector.h>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

namespace pybind11::detail
{
  template <typename Type, typename Alloc>
  struct type_caster<eastl::vector<Type, Alloc>>
    : list_caster<eastl::vector<Type, Alloc>, Type> {};
}

#endif
