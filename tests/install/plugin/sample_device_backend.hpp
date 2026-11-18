// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device_backend.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace sample
{

/**
 * @brief The backend of the sample device, named "sample". It has one
 * device, numbered 0.
 */
class sample_device_backend final
	: public rexlib::device_backend
{
public:
	std::string get_name() const override;
	rexlib::version get_version() const override;
	void enumerate_devices(std::vector<std::size_t> &ids) const override;
	bool get_device_properties(
		std::size_t id,
		rexlib::device_properties &desc
	) const override;
	std::shared_ptr<rexlib::device>
	create_device(std::size_t id) const override;
};

} // namespace sample
