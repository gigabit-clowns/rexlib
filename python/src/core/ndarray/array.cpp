// SPDX-License-Identifier: GPL-3.0-only

#include "array.hpp"

namespace rexlib
{

array_class declare_array(pybind11::module_ &m)
{
	return array_class(m, "Array");
}

} // namespace rexlib
