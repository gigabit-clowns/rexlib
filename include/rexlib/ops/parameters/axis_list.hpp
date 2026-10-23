// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace ops
{

/**
 * @brief The axes an operation acts along.
 *
 * Explicit and non negative. An axis referred from the end is resolved by
 * whoever holds the array, since resolving it needs the rank.
 */
using axis_list = std::vector<std::size_t>;

} // namespace ops
} // namespace rexlib
