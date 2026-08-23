// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/device_backend.hpp>

#include <trompeloeil.hpp>

namespace rex
{

class mock_device_backend final
	: public device_backend
{
public:
	MAKE_CONST_MOCK0(get_name, std::string (), override);
	MAKE_CONST_MOCK0(get_version, version (), override);
	MAKE_CONST_MOCK1(enumerate_devices, void (std::vector<std::size_t>&), override);
	MAKE_CONST_MOCK2(get_device_properties, bool (std::size_t, device_properties&), override);
	MAKE_CONST_MOCK1(create_device, std::shared_ptr<device> (std::size_t), override);
};

} // namespace rex
