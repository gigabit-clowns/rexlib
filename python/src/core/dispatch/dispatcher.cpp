// SPDX-License-Identifier: GPL-3.0-only

#include "dispatcher.hpp"

#include <rexlib/core/dispatch/program_manager.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

dispatcher_class declare_dispatcher(nb::module_ &m)
{
	return dispatcher_class(m, "Dispatcher");
}

void define_dispatcher(nb::module_ &m)
{
	m.def(
		"make_eager_dispatcher",
		&make_eager_dispatcher,
		nb::arg("program_manager")
	);
}

} // namespace rexlib
