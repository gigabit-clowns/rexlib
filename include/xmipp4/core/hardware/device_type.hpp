// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"

namespace rex
{

/**
 * @brief Broad classification of compute devices.
 */
enum class device_type
{
	/// Catch-all for devices that do not fit any of the following
	/// categories.
	unknown = -1,
	/// CPU of the host node.
	cpu,
	/// A discrete Graphics Processing Unit (GPU), with its own dedicated
	/// memory.
	gpu,
	/// An integrated Graphics Processing Unit (iGPU), typically sharing
	/// memory with the host CPU.
	integrated_gpu,
};

REXLIB_API
const char* to_string(device_type type) noexcept;

} // namespace rex
