// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "arithmetic.hpp"
#include "cast.hpp"
#include "creation.hpp"
#include "transfer.hpp"

namespace xmipp4
{
namespace functional
{

void bind_functional(pybind11::module_ &m)
{
	bind_arithmetic(m);
	bind_cast(m);
	bind_creation(m);
	bind_transfer(m);
}

} // namespace functional
} // namespace xmipp4
