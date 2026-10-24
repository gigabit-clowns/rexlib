// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief The number of values a sequence holds.
 */
class sequence_length
{
public:
	/**
	 * @brief Construct the length of a sequence.
	 *
	 * @param count The number of values.
	 */
	REXLIB_API
	explicit sequence_length(std::size_t count) noexcept;

	/**
	 * @brief Get the number of values.
	 *
	 * @return std::size_t The count.
	 */
	REXLIB_API
	std::size_t get_count() const noexcept;

private:
	std::size_t m_count;
};

} // namespace ops
} // namespace rexlib
