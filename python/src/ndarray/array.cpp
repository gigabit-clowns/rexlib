// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

namespace xmipp4
{
namespace ndarray
{

array_class declare_array(pybind11::module_ &m)
{
	return array_class(m, "Array");
}

} // namespace ndarray
} // namespace xmipp4
