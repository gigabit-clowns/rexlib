// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/device_backend.hpp>

namespace xmipp4 
{
namespace hardware
{

class device_manager;

class cuda_device_backend final
	: public device_backend
{
public:
	std::string get_name() const override;
	version get_version() const override;
	void enumerate_devices(std::vector<std::size_t> &ids) const override;
	bool get_device_properties(
		std::size_t id,
		device_properties &desc
	) const override;

	std::shared_ptr<device> 
	create_device(std::size_t id) override;

	static bool register_at(device_manager &manager);

}; 

} // namespace hardware
} // namespace xmipp4
