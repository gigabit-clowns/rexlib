// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "array.hpp"
#include "array_descriptor.hpp"

namespace xmipp4
{
namespace ndarray
{

void bind_ndarray(pybind11::module_ &m)
{
	declare_array(m);
	auto array_descriptor = declare_array_descriptor(m);

	define_array_descriptor(array_descriptor, m);
}

} // namespace ndarray
} // namespace xmipp4
