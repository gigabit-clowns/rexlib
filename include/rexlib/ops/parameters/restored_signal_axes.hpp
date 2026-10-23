// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_list.hpp"

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief The axes an inverse real transform acts along, and the extent of
 * the last of them.
 *
 * A half spectrum does not tell how many samples its last axis had, so the
 * extent is named. Whether the values are admissible is for the shape rule
 * of the operation to decide.
 */
class restored_signal_axes
{
public:
	/**
	 * @brief Construct the parameters of an inverse real transform.
	 *
	 * @param axes The axes transformed, the last one named being restored.
	 * @param extent The number of samples along the restored axis.
	 */
	REXLIB_API
	restored_signal_axes(axis_list axes, std::size_t extent) noexcept;

	/**
	 * @brief Get the axes transformed.
	 *
	 * @return span<const std::size_t> The axes, in the order given.
	 */
	REXLIB_API
	span<const std::size_t> get_axes() const noexcept;

	/**
	 * @brief Get the number of samples along the restored axis.
	 *
	 * @return std::size_t The extent.
	 */
	REXLIB_API
	std::size_t get_extent() const noexcept;

private:
	axis_list m_axes;
	std::size_t m_extent;
};

} // namespace ops
} // namespace rexlib
