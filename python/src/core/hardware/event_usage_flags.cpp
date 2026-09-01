// SPDX-License-Identifier: GPL-3.0-only

#include "event_usage_flags.hpp"

#include <rexlib/core/hardware/event_usage_flags.hpp>

#include <pybind11/operators.h>

#include <sstream>

namespace rexlib
{

namespace py = pybind11;

static std::string to_repr(const event_usage_flags &f)
{
	std::ostringstream oss;
	oss << "EventUsageFlags(bits=" << f.get_bits() << ")";
	return oss.str();
}

event_usage_flag_bits_class declare_event_usage_flag_bits(pybind11::module_ &m)
{
	return event_usage_flag_bits_class(m, "EventUsageFlagBits");
}

void define_event_usage_flag_bits(event_usage_flag_bits_class &c)
{
	c
		.value("host_query", event_usage_flag_bits::host_query)
		.value("host_wait", event_usage_flag_bits::host_wait)
		.value("device_wait", event_usage_flag_bits::device_wait)
		.value("cross_device_wait", event_usage_flag_bits::cross_device_wait);
}

event_usage_flags_class declare_event_usage_flags(pybind11::module_ &m)
{
	return event_usage_flags_class(m, "EventUsageFlags");
}

void define_event_usage_flags(event_usage_flags_class &c)
{
	c
		.def(py::init<event_usage_flag_bits>())
		.def(py::init<event_usage_flags::underlying_type>())
		.def(py::self | py::self)
		.def(py::self & py::self)
		.def(py::self ^ py::self)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("contains", &event_usage_flags::contains)
		.def_property_readonly("bits", &event_usage_flags::get_bits)
		.def("__repr__", &to_repr);
}

} // namespace rexlib
