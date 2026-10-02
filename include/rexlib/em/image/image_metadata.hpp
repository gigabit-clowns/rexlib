// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace em
{

/**
 * @brief What a file states beyond its shape and data type, such as how its
 * samples map onto physical space.
 *
 * It carries no fields yet, so every instance states nothing.
 */
class image_metadata
{
public:
	/**
	 * @brief Construct metadata stating nothing.
	 */
	REXLIB_API
	image_metadata() noexcept;

	REXLIB_API
	image_metadata(const image_metadata &other);
	REXLIB_API
	image_metadata(image_metadata &&other) noexcept;
	REXLIB_API
	~image_metadata();

	REXLIB_API
	image_metadata& operator=(const image_metadata &other);
	REXLIB_API
	image_metadata& operator=(image_metadata &&other) noexcept;
};

} // namespace em
} // namespace rexlib
