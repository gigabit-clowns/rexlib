// SPDX-License-Identifier: GPL-3.0-only

#include "event.hpp"

#include <rexlib/core/hardware/event.hpp>

namespace rexlib
{

namespace py = pybind11;

event_class declare_event(pybind11::module_ &m)
{
	return event_class(m, "Event");
}

void define_event(event_class &c)
{
	c
		.def_property_readonly("supported_usage", &event::get_supported_usage)
		.def("wait", &event::wait)
		.def_property_readonly("is_signaled", &event::is_signaled);
}

} // namespace rexlib
