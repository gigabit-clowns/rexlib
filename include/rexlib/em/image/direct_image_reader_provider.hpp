// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_read_format_manager;

/**
 * @brief A provider that opens a file every time it is asked for one.
 *
 * It keeps nothing, so a path asked for twice is opened twice and the two
 * readers are unrelated. That is what makes it the plain one: there is no
 * capacity to size, no eviction to reason about, and a test counting how
 * often a file is opened counts exactly the calls it made.
 *
 * It is also the only reader provider in the library that knows what a
 * format is. @ref caching_image_reader_provider is a decorator over another
 * provider rather than a second opener, so the code that turns a path into a
 * reader lives here alone.
 */
class direct_image_reader_provider final
	: public image_reader_provider
{
public:
	/**
	 * @brief Construct a provider opening through a format manager.
	 *
	 * @param formats The formats a file may be opened with.
	 * @throws std::invalid_argument If @p formats is null.
	 */
	REXLIB_API
	explicit direct_image_reader_provider(
		std::shared_ptr<const image_read_format_manager> formats
	);

	REXLIB_API
	~direct_image_reader_provider() override;

	/**
	 * @brief Open a file and return a reader over it.
	 *
	 * @param path Path to the file to read.
	 * @return std::shared_ptr<const image_reader> The reader, never null and
	 * never one returned before.
	 * @throws invalid_operation_error If no format recognizes the file.
	 * @throws image_format_error If the file is malformed or truncated.
	 */
	REXLIB_API
	std::shared_ptr<const image_reader>
	acquire(const std::string &path) override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<const image_read_format_manager> m_formats;
};

} // namespace em
} // namespace rexlib
