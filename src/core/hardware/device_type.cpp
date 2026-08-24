// SPDX-License-Identifier: GPL-3.0-only

#include "rex/core/hardware/device_type.hpp"

namespace rexlib
{

const char* to_string(device_type type) noexcept
{
	switch (type)
	{
	case device_type::unknown: return "unknown";
	case device_type::cpu: return "CPU";
	case device_type::gpu: return "GPU";
	case device_type::integrated_gpu: return "iGPU";
	default: return "";
	}
}

} // namespace rexlib
