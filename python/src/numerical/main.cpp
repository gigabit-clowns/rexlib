// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "numerical_type.hpp"

namespace xmipp4
{
namespace numerical
{

void bind_numerical(pybind11::module_ &m)
{
	bind_numerical_type(m);
}

} // namespace numerical
} // namespace xmipp4
