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
 * @brief The axes a reduction acts along, and whether its result keeps
 * them with an extent of one.
 *
 * The axes keep the order they were given in. Whether they are admissible
 * is for the shape rule of the operation to decide.
 */
class reduction_axes
{
public:
	/**
	 * @brief Construct the parameters of a reduction.
	 *
	 * @param axes The axes reduced.
	 * @param keep_dimensions Whether the result keeps the reduced axes
	 * with an extent of one.
	 */
	REXLIB_API
	explicit reduction_axes(
		axis_list axes,
		bool keep_dimensions = false
	) noexcept;

	/**
	 * @brief Get the axes reduced.
	 *
	 * @return span<const std::size_t> The axes, in the order given.
	 */
	REXLIB_API
	span<const std::size_t> get_axes() const noexcept;

	/**
	 * @brief Check whether the result keeps the reduced axes.
	 *
	 * @return true The reduced axes are kept with an extent of one.
	 * @return false The reduced axes are dropped.
	 */
	REXLIB_API
	bool get_keep_dimensions() const noexcept;

private:
	axis_list m_axes;
	bool m_keep_dimensions;
};

} // namespace ops
} // namespace rexlib
