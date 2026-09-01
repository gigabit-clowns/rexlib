// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <limits>
#include <ostream>
#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief Address of one element inside an image dataset.
 *
 * The pair of a path to a dataset and a zero based index along the slowest
 * axis of that dataset. The index @ref no_position addresses the dataset as a
 * whole rather than one of its elements, which is how a file holding a single
 * image or volume is named.
 *
 * The path is held as a string rather than as a filesystem path so that this
 * type stays free of the private dependencies of the library.
 *
 * @see parse_image_location
 */
class image_location
{
public:
	/**
	 * @brief Index addressing the dataset rather than one of its elements.
	 */
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t no_position =
		std::numeric_limits<std::size_t>::max();

	/**
	 * @brief Construct a location from its components.
	 *
	 * @param path Path to the dataset holding the element.
	 * @param position Zero based index of the element along the slowest axis
	 * of the dataset, or @ref no_position to address the whole dataset.
	 */
	REXLIB_API
	explicit image_location(
		std::string path,
		std::size_t position = no_position
	);

	/**
	 * @brief Construct a location with an empty path addressing no element.
	 */
	REXLIB_API
	image_location() noexcept;
	REXLIB_API
	image_location(const image_location &other);
	REXLIB_API
	image_location(image_location &&other) noexcept;
	REXLIB_API
	~image_location();

	REXLIB_API
	image_location& operator=(const image_location &other);
	REXLIB_API
	image_location& operator=(image_location &&other) noexcept;

	/**
	 * @brief Get the hash value for this location.
	 *
	 * @return std::size_t The hash value.
	 */
	REXLIB_API
	std::size_t hash() const noexcept;

	/**
	 * @brief Get the path to the dataset.
	 *
	 * @return const std::string& Reference to the stored path. The reference
	 * is valid for the lifetime of this @ref image_location and is
	 * invalidated by assignment to or destruction of the object.
	 */
	REXLIB_API
	const std::string& get_path() const noexcept;

	/**
	 * @brief Get the index of the element within the dataset.
	 *
	 * @return std::size_t The zero based index along the slowest axis of the
	 * dataset, or @ref no_position when the location addresses the dataset
	 * as a whole.
	 */
	REXLIB_API
	std::size_t get_position_in_stack() const noexcept;

	friend bool
	operator==(const image_location &lhs, const image_location &rhs) noexcept
	{
		return
			lhs.get_position_in_stack() == rhs.get_position_in_stack() &&
			lhs.get_path() == rhs.get_path();
	}

	friend bool
	operator!=(const image_location &lhs, const image_location &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	friend bool
	operator<(const image_location &lhs, const image_location &rhs) noexcept
	{
		if (lhs.get_path() < rhs.get_path())
		{
			return true;
		}
		else if (lhs.get_path() == rhs.get_path())
		{
			return lhs.get_position_in_stack() < rhs.get_position_in_stack();
		}
		return false;
	}

	friend bool
	operator<=(const image_location &lhs, const image_location &rhs) noexcept
	{
		return !(rhs < lhs);
	}

	friend bool
	operator>(const image_location &lhs, const image_location &rhs) noexcept
	{
		return rhs < lhs;
	}

	friend bool
	operator>=(const image_location &lhs, const image_location &rhs) noexcept
	{
		return !(lhs < rhs);
	}

	friend std::ostream&
	operator<<(std::ostream &os, const image_location &location)
	{
		if (location.get_position_in_stack() != no_position)
		{
			os << (location.get_position_in_stack() + 1) << '@';
		}
		return os << location.get_path();
	}

private:
	std::string m_path;
	std::size_t m_position_in_stack;
};

/**
 * @brief Parse an image location from its string representation.
 *
 * The representation is expected to be:
 * <index>@<path> (a one based index into the dataset)
 * <path> (addresses the dataset as a whole)
 *
 * The index is one based to match the convention used by the star files of
 * the field, while @ref image_location::get_position_in_stack is zero based;
 * this function bridges the two. An index of zero is therefore not a valid
 * representation.
 *
 * @param text Appropriately formatted string with the location.
 * @param result Output image_location object.
 * @return true The string was parsed successfully and the result was written.
 * @return false The string was not parsed and the result was not written.
 *
 * @see to_string
 */
REXLIB_API
bool parse_image_location(const std::string &text, image_location &result);

/**
 * @brief Write an image location as its string representation.
 *
 * The exact inverse of @ref parse_image_location: a location addressing the
 * whole dataset is written as a bare path, and any other location is prefixed
 * with its one based index.
 *
 * @param location The location to be written.
 * @return std::string The string representation.
 */
REXLIB_API
std::string to_string(const image_location &location);

} // namespace em
} // namespace rexlib

namespace std
{

template<>
struct hash<rexlib::em::image_location>
{
	std::size_t operator()(
		const rexlib::em::image_location &location
	) const noexcept
	{
		return location.hash();
	}
};

} // namespace std
