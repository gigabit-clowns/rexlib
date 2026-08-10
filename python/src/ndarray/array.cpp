// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

#include <xmipp4/core/ndarray/array.hpp>

namespace xmipp4
{
namespace ndarray
{

namespace py = pybind11;

void bind_array(pybind11::module_ &m)
{
	py::class_<array>(m, "Array");
}

} // namespace ndarray
} // namespace xmipp4
