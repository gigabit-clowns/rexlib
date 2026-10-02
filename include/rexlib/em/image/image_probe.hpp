// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief The path, extension and leading bytes of a file, from which a
 * format can tell whether it fits the file.
 *
 * It reads the leading bytes once, when it is constructed, and never
 * changes afterwards, so one probe can be shown to any number of formats and
 * shared between threads.
 *
 * A file that does not exist yields a probe with no leading bytes rather
 * than an error, so a probe can name a file yet to be created, and what
 * fits it then rests on the extension alone.
 */
class image_probe
{
public:
	/**
	 * @brief Most bytes a probe reads off the front of a file.
	 */
	static
	REXLIB_INLINE_CONST_CONSTEXPR std::size_t max_leading_bytes = 4096;

	/**
	 * @brief Construct a probe by reading the leading bytes of a file.
	 *
	 * Reads at most @ref max_leading_bytes bytes, fewer when the file is
	 * shorter and none when it does not exist or can not be read.
	 *
	 * @param path Path to the file to probe.
	 */
	REXLIB_API
	explicit image_probe(std::string path);

	REXLIB_API
	image_probe(const image_probe &other);
	REXLIB_API
	image_probe(image_probe &&other) noexcept;
	REXLIB_API
	~image_probe();

	REXLIB_API
	image_probe& operator=(const image_probe &other);
	REXLIB_API
	image_probe& operator=(image_probe &&other) noexcept;

	/**
	 * @brief Get the path of the probed file.
	 *
	 * @return const std::string& The path, as it was given.
	 */
	REXLIB_API
	const std::string& get_path() const noexcept;

	/**
	 * @brief Get the extension of the probed file.
	 *
	 * Folded to lower case and including the leading dot. Empty when the
	 * file name has no extension.
	 *
	 * @return const std::string& The extension.
	 */
	REXLIB_API
	const std::string& get_extension() const noexcept;

	/**
	 * @brief Get the leading bytes of the probed file.
	 *
	 * At most @ref max_leading_bytes bytes, and fewer when the file is
	 * shorter. They are the front of the file and nothing more: what counts
	 * as its header is up to each format.
	 *
	 * @return span<const byte> The bytes. Empty when the file does not
	 * exist, is empty, or could not be read.
	 */
	REXLIB_API
	span<const byte> get_leading_bytes() const noexcept;

	/**
	 * @brief Check whether the probed file exists and could be read.
	 *
	 * @return true The file exists and its leading bytes were read.
	 * @return false The file does not exist or could not be read, in which
	 * case @ref get_leading_bytes is empty.
	 */
	REXLIB_API
	bool exists() const noexcept;

private:
	std::string m_path;
	std::string m_extension;
	std::vector<byte> m_leading_bytes;
	bool m_exists;
};

} // namespace em
} // namespace rexlib
