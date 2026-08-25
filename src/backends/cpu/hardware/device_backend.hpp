// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device_backend.hpp>

namespace rexlib 
{

class device_manager;

namespace cpu
{

/**
 * @brief Implementation of the @ref device_backend interface to retrieve the 
 * CPU device.
 */
class device_backend final
	: public rexlib::device_backend
{
public:
	std::string get_name() const override;
	version get_version() const override;

	void enumerate_devices(std::vector<std::size_t> &ids) const override;

	bool get_device_properties(
		std::size_t id, 
		device_properties &desc
	) const override;

	std::shared_ptr<rexlib::device>
	create_device(std::size_t id) const override;

	static bool register_at(rexlib::device_manager &manager);
}; 

} // namespace cpu
} // namespace rexlib
