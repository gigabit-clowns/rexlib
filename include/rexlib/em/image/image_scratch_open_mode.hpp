// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace em
{

/**
 * @brief How a scratch treats what its storage holds when it is
 * constructed.
 */
enum class image_scratch_open_mode
{
	/// The scratch starts with nothing loaded. The storage is not read.
	empty,
	/// The scratch keeps what an earlier scratch of the same layout loaded
	/// into the same storage.
	resumed
};

} // namespace em
} // namespace rexlib
