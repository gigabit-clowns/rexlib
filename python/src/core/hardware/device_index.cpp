// SPDX-License-Identifier: GPL-3.0-only

#include "device_index.hpp"

#include <rexlib/core/hardware/device_index.hpp>

#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <tuple>

namespace rexlib
{

namespace nb = nanobind;

static std::string to_string(const device_index &l)
{
	std::ostringstream oss;
	oss << l;
	return oss.str();
}

static std::string to_repr(const device_index &l)
{
	std::ostringstream oss;
	oss << "DeviceIndex(backend=\"" << l.get_backend_name() << "\", " 
		<< "id=" << l.get_device_id() << ")";
	return oss.str();
}

static device_index from_string(const std::string &str)
{
	device_index result;
	if (!parse_device_index(str, result))
	{
		std::ostringstream oss;
		oss << "Invalid device_index syntax \"" << str << "\"\n"
			<< R"(Expected syntax "backend_name:device_id" or "backend_name")";
		throw std::invalid_argument(oss.str());
	}
	return result;
}



device_index_class declare_device_index(nb::module_ &m)
{
	return device_index_class(m, "DeviceIndex");
}

void define_device_index(device_index_class &c)
{
	c
		.def(
			nb::init<const std::string&, std::size_t>(),
			nb::arg("backend"), nb::arg("id")
		)
		.def_static("from_string", &from_string, nb::arg("text"))
		.def(nb::self == nb::self)
		.def(nb::self != nb::self)
		.def(nb::self < nb::self)
		.def(nb::self <= nb::self)
		.def(nb::self > nb::self)
		.def(nb::self >= nb::self)
		.def("__str__", &to_string)
		.def("__repr__", &to_repr)
		.def_prop_ro("backend", &device_index::get_backend_name)
		.def_prop_ro("id", &device_index::get_device_id)
		.def(
			"__getstate__",
			[](const device_index &l)
			{
				return std::make_tuple(
					l.get_backend_name(),
					l.get_device_id()
				);
			}
		)
		.def(
			"__setstate__",
			[](
				device_index &l,
				const std::tuple<std::string, std::size_t> &state
			)
			{
				new (&l) device_index(
					std::get<0>(state),
					std::get<1>(state)
				);
			}
		);

	c.attr("__hash__") = nb::none();
}

} // namespace rexlib
