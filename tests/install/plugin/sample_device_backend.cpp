// SPDX-License-Identifier: GPL-3.0-only

#include "sample_device_backend.hpp"

#include "sample_device.hpp"

#include <rexlib/core/hardware/device_type.hpp>

#include <stdexcept>

namespace sample
{

std::string sample_device_backend::get_name() const
{
	return "sample";
}

rexlib::version sample_device_backend::get_version() const
{
	return rexlib::version(
		SAMPLE_VERSION_MAJOR,
		SAMPLE_VERSION_MINOR,
		SAMPLE_VERSION_PATCH
	);
}

void sample_device_backend::enumerate_devices(
	std::vector<std::size_t> &ids
) const
{
	ids = { 0 };
}

bool sample_device_backend::get_device_properties(
	std::size_t id,
	rexlib::device_properties &desc
) const
{
	if (id != 0)
	{
		return false;
	}

	desc.set_type(rexlib::device_type::cpu);
	desc.set_name("sample");
	return true;
}

std::shared_ptr<rexlib::device>
sample_device_backend::create_device(std::size_t id) const
{
	if (id != 0)
	{
		throw std::invalid_argument(
			"The sample backend has one device, numbered 0."
		);
	}

	return std::make_shared<sample_device>();
}

} // namespace sample
