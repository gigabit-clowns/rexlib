// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief How the samples of a page are encoded when it is written.
 */
enum class tiff_compression
{
	none,
	lzw,
};

} // namespace tiff
} // namespace em
} // namespace rexlib
