// SPDX-License-Identifier: GPL-3.0-only

#include "device_properties.hpp"

#include <rexlib/core/hardware/device_properties.hpp>

#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

#include <cstddef>
#include <string>
#include <tuple>

namespace rexlib
{

namespace nb = nanobind;

device_properties_class declare_device_properties(nb::module_ &m)
{
	return device_properties_class(m, "DeviceProperties");
}

void define_device_properties(device_properties_class &c)
{
	c
		.def(nb::init<>())
		.def_prop_rw(
			"type", 
			&device_properties::get_type, 
			&device_properties::set_type
		)
		.def_prop_rw(
			"name",
			&device_properties::get_name,
			&device_properties::set_name
		)
		.def_prop_rw(
			"physical_location",
			&device_properties::get_physical_location,
			&device_properties::set_physical_location
		)
		.def_prop_rw(
			"total_memory_bytes",
			&device_properties::get_total_memory_bytes,
			&device_properties::set_total_memory_bytes
		)
		.def_prop_rw(
			"optimal_data_alignment",
			&device_properties::get_optimal_data_alignment,
			&device_properties::set_optimal_data_alignment
		)
		.def(
			"__getstate__",
			[](const device_properties &l)
			{
				return std::make_tuple(
					l.get_type(),
					l.get_name(),
					l.get_physical_location(),
					l.get_total_memory_bytes(),
					l.get_optimal_data_alignment()
				);
			}
		)
		.def(
			"__setstate__",
			[](
				device_properties &l,
				const std::tuple<
					device_type,
					std::string,
					std::string,
					std::size_t,
					std::size_t
				> &state
			)
			{
				new (&l) device_properties();
				l.set_type(std::get<0>(state));
				l.set_name(std::get<1>(state));
				l.set_physical_location(std::get<2>(state));
				l.set_total_memory_bytes(std::get<3>(state));
				l.set_optimal_data_alignment(std::get<4>(state));
			}
		);

}

} // namespace rexlib
