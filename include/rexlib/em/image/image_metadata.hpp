// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief How the samples of a file map onto physical space.
 *
 * Holds the two quantities every electron microscopy format carries and
 * every geometric operation needs. Both are given per axis, ordered as the
 * axes of the file descriptor are, so the slowest axis comes first.
 *
 * A quantity that the file does not state is left empty rather than
 * defaulted, so that a caller can tell an unstated sampling from a sampling
 * of one. Format specific header fields are not modelled here; a reader that
 * needs to expose them does so on its own.
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
	 * @brief Construct metadata from its components.
	 *
	 * @param sampling Distance between consecutive samples along each axis,
	 * in angstrom. Empty when the file does not state it.
	 * @param origin Position of the first sample along each axis, in
	 * angstrom. Empty when the file does not state it.
	 */
	REXLIB_API
	image_metadata(
		std::vector<double> sampling,
		std::vector<double> origin
	);

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
	 * @param[out] sampling Output parameter receiving the distances, in
	 * angstrom, ordered as the axes of the file. Cleared before being
	 * populated, and left empty when the file does not state a sampling.
	 */
	REXLIB_API
	void get_sampling(std::vector<double> &sampling) const;

	/**
	 * @brief Get the position of the first sample on each axis.
	 *
	 * @param[out] origin Output parameter receiving the positions, in
	 * angstrom, ordered as the axes of the file. Cleared before being
	 * populated, and left empty when the file does not state an origin.
	 */
	REXLIB_API
	void get_origin(std::vector<double> &origin) const;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<double> m_sampling;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<double> m_origin;
};

} // namespace em
} // namespace rexlib
