// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "array.hpp"

namespace xmipp4
{
namespace multidimensional
{

namespace py = pybind11;

void bind_multidimensional(pybind11::module_ &m)
{
	bind_array(m);
}

} // namespace multidimensional
} // namespace xmipp4
