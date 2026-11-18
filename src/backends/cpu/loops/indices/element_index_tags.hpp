// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace cpu
{

/**
 * @brief Tag requesting a traversal that hands its kernel no index.
 */
struct no_index_tag
{
};

/**
 * @brief Tag requesting a traversal that hands its kernel the linear index of
 * every element.
 *
 * The linear index counts the elements of the index space in row-major order,
 * its last axis varying fastest, whatever order they are visited in.
 */
struct linear_index_tag
{
};

/**
 * @brief Tag requesting a traversal that hands its kernel the coordinates of
 * every element, one per axis of the index space.
 */
struct multidimensional_index_tag
{
};

} // namespace cpu
} // namespace rexlib
