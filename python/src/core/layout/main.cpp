// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "index_table.hpp"

namespace rexlib
{

void bind_layout(pybind11::module_ &m)
{
	auto index_table = declare_index_table(m);

	define_index_table(index_table);
}

} // namespace rexlib
