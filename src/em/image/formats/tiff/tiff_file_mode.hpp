// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief What a TIFF file is opened for.
 *
 * A classic file addresses itself with 32 bit offsets and so ends at 4 GiB,
 * and a BigTIFF one uses 64 bit offsets. Which of the two a file is is
 * settled when it is created, and read off it when it is opened.
 */
enum class tiff_file_mode
{
	read,
	create,
	create_big,
};

} // namespace tiff
} // namespace em
} // namespace rexlib
