// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "dispatcher.hpp"
#include "execution_context.hpp"
#include "program_manager.hpp"

namespace rexlib
{

void bind_dispatch(pybind11::module_ &m)
{
	declare_dispatcher(m);
	auto execution_context = declare_execution_context(m);
	declare_program_manager(m);

	define_dispatcher(m);
	define_execution_context(execution_context);
	define_program_manager(m);
}

} // namespace rexlib
