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
 * @brief The axes an operation acts along.
 *
 * The axes keep the order they were given in, for an operation that gives
 * the last of them a role of its own. Whether they are admissible is for
 * the shape rule of the operation to decide.
 */
class axis_selection
{
public:
	/**
	 * @brief Construct a selection of axes.
	 *
	 * @param axes The axes the operation acts along.
	 */
	REXLIB_API
	explicit axis_selection(axis_list axes) noexcept;

	/**
	 * @brief Get the axes the operation acts along.
	 *
	 * @return span<const std::size_t> The axes, in the order given.
	 */
	REXLIB_API
	span<const std::size_t> get_axes() const noexcept;

private:
	axis_list m_axes;
};

} // namespace ops
} // namespace rexlib
