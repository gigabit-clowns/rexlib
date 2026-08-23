// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/device_backend.hpp>

namespace rex 
{

class device_manager;

namespace cpu
{

/**
 * @brief Implementation of the @ref device_backend interface to retrieve the 
 * CPU device.
 */
class device_backend final
	: public xmipp4::device_backend
{
public:
	std::string get_name() const override;
	version get_version() const override;

	void enumerate_devices(std::vector<std::size_t> &ids) const override;

	bool get_device_properties(
		std::size_t id, 
		device_properties &desc
	) const override;

	std::shared_ptr<xmipp4::device>
	create_device(std::size_t id) const override;

	static bool register_at(xmipp4::device_manager &manager);
}; 

} // namespace cpu
} // namespace rex
