// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief How the samples of a file map onto physical space.
 *
 * Holds the sampling, which is what turns an index into a distance and so
 * what every geometric operation needs. It is given per axis, ordered as
 * the axes of the file descriptor are, so the slowest axis comes first.
 *
 * A file that does not state it leaves it empty rather than defaulted, so
 * that a caller can tell an unstated sampling from a sampling of one.
 * Format specific header fields are not modelled here; a reader that needs
 * to expose them does so on its own.
 */
class image_metadata
{
public:
	/**
	 * @brief Construct metadata stating nothing.
	 */
	REXLIB_API
	image_metadata() noexcept;

	/**
	 * @brief Construct metadata stating a sampling.
	 *
	 * @param sampling Distance between consecutive samples along each axis,
	 * in angstrom. Empty when the file does not state it.
	 */
	REXLIB_API
	explicit image_metadata(std::vector<double> sampling);

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

	/**
	 * @brief Get the distance between consecutive samples on each axis.
	 *
	 * @return span<const double> The distances, in angstrom, ordered as the
	 * axes of the file. Empty when the file does not state a sampling. It
	 * refers to storage owned by this object, which assignment to or
	 * destruction of it invalidates.
	 */
	REXLIB_API
	span<const double> get_sampling() const noexcept;

private:
	std::vector<double> m_sampling;
};

} // namespace em
} // namespace rexlib
