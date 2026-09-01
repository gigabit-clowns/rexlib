// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_access_flags.hpp"

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief What a reader costs, as opposed to what it can do.
 *
 * A reader serves any in-bounds region correctly whatever its traits say.
 * These describe the price of doing so, which is what a consumer needs in
 * order to schedule its reads well. They belong to an open reader rather
 * than to a format, since the same file read through a mapping and read
 * through positioned reads does not have the same traits.
 */
class image_access_traits
{
public:
	/**
	 * @brief Construct traits describing nothing.
	 *
	 * The granularity is empty, no flag is set and the preferred data type
	 * is unknown.
	 */
	REXLIB_API
	image_access_traits() noexcept;

	/**
	 * @brief Construct traits from their components.
	 *
	 * @param granularity Extents of the smallest region the reader can read
	 * for the price of one element. Its size must equal the rank of the
	 * file.
	 * @param flags The advertised properties.
	 * @param preferred_data_type The data type a read produces without
	 * converting.
	 */
	REXLIB_API
	image_access_traits(
		span<const std::size_t> granularity,
		image_access_flags flags,
		numerical_type preferred_data_type
	);

	REXLIB_API
	image_access_traits(const image_access_traits &other);
	REXLIB_API
	image_access_traits(image_access_traits &&other) noexcept;
	REXLIB_API
	~image_access_traits();

	REXLIB_API
	image_access_traits& operator=(const image_access_traits &other);
	REXLIB_API
	image_access_traits& operator=(image_access_traits &&other) noexcept;

	/**
	 * @brief Get the extents of the smallest region worth reading.
	 *
	 * Reading a region smaller than this along any axis costs the same as
	 * reading one of this size, because that is the unit the format decodes.
	 * A format storing plain elements reports all ones; a chunked format
	 * reports its chunk extents; a format that codes a whole plane at a time
	 * reports one along the slowest axis and the full extent along the rest.
	 *
	 * Read this to choose what to cache and how much scratch memory a read
	 * needs. A region is cheap to read along an axis exactly when the
	 * granularity along it is one.
	 *
	 * @param[out] granularity Output parameter receiving the extents.
	 * Cleared before being populated.
	 */
	REXLIB_API
	void get_granularity(std::vector<std::size_t> &granularity) const;

	/**
	 * @brief Get the advertised properties of the reader.
	 *
	 * @return image_access_flags The properties.
	 */
	REXLIB_API
	image_access_flags get_flags() const noexcept;

	/**
	 * @brief Get the data type a read produces without converting.
	 *
	 * A read into a destination of this type copies and never converts, so
	 * this is the cheapest type to ask a reader for. It is the type the
	 * reader produces rather than the encoding on the storage, which the
	 * numerical types can not always name: a format storing pairs of
	 * sixteen bit integers as complex numbers reports complex_float32.
	 *
	 * @return numerical_type The data type.
	 */
	REXLIB_API
	numerical_type get_preferred_data_type() const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_granularity;
	image_access_flags m_flags;
	numerical_type m_preferred_data_type;
};

} // namespace em
} // namespace rexlib
