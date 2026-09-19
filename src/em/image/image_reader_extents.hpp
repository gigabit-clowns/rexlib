// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class image_reader;

/**
 * @brief Copy the extents a reader reports.
 *
 * @ref image_reader::get_extents refers to storage the reader owns, which a
 * caller holding the reader in a temporary outlives. This owns what it
 * returns.
 *
 * @param reader The reader to ask.
 * @return std::vector<std::size_t> The extents of the file.
 */
std::vector<std::size_t> copy_extents(const image_reader &reader);

/**
 * @brief Copy the extents of one image or volume of a reader.
 *
 * The trailing @ref image_reader::get_core_rank extents, so the axes a file
 * stacks along are left out and what remains is the shape of a single image
 * or volume.
 *
 * @param reader The reader to ask.
 * @return std::vector<std::size_t> The extents of one image or volume.
 */
std::vector<std::size_t> copy_core_extents(const image_reader &reader);

} // namespace em
} // namespace rexlib
