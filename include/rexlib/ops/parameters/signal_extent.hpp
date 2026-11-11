// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief The number of samples along each axis of a signal.
 *
 * A half spectrum does not tell how many samples the signal it stands for
 * had, so an operation producing one names the extent. Whether the value is
 * admissible is for the shape rule of the operation to decide.
 */
class signal_extent
{
public:
	/**
	 * @brief Construct the extent of a signal.
	 *
	 * @param extent The number of samples along each axis.
	 */
	REXLIB_API
	explicit signal_extent(std::size_t extent) noexcept;

	/**
	 * @brief Get the number of samples along each axis.
	 *
	 * @return std::size_t The extent.
	 */
	REXLIB_API
	std::size_t get_extent() const noexcept;

private:
	std::size_t m_extent;
};

} // namespace ops
} // namespace rexlib
