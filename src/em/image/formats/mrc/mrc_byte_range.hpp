// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief A stretch of a mapping, in bytes from its start.
 *
 * There is no invariant to hold, so the two numbers are named rather than
 * wrapped: a stretch is whatever a caller says it is, and what is mapped is
 * only known where it is used.
 */
struct mrc_byte_range
{
	std::size_t byte_offset;
	std::size_t byte_size;
};

} // namespace mrc
} // namespace em
} // namespace rexlib
