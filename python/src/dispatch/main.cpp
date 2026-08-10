// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "dispatcher.hpp"
#include "program_manager.hpp"
#include "execution_context.hpp"

namespace xmipp4
{
namespace dispatch
{

void bind_dispatch(pybind11::module_ &m)
{
	bind_dispatcher(m);
	bind_program_manager(m);
	bind_execution_context(m);
}

} // namespace dispatch
} // namespace xmipp4
