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
 * @brief What every format is shown when it is asked whether it fits a file.
 *
 * A probe is built once per open and handed to each candidate format, so the
 * leading bytes of the file are read once however many formats are
 * registered, and no format opens the file just to decline it.
 *
 * It is filled when it is constructed and never changes afterwards, which is
 * what lets it be shared. A file that does not exist yields a probe with no
 * leading bytes rather than an error, since opening a dataset for writing
 * names a file that is not there yet and the decision then rests on the
 * extension alone.
 */
class image_probe
{
public:
	/**
	 * @brief Number of leading bytes a probe reads.
	 */
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t header_size = 1024;

	/**
	 * @brief Construct a probe by reading the leading bytes of a file.
	 *
	 * Reads at most @ref header_size bytes, fewer when the file is shorter
	 * and none when it does not exist or can not be read.
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
	 * Folded to lower case and including the leading dot, so that a format
	 * matches on it without spelling the comparison out itself. Empty when
	 * the file name has no extension.
	 *
	 * @return const std::string& The extension.
	 */
	REXLIB_API
	const std::string& get_extension() const noexcept;

	/**
	 * @brief Get the leading bytes of the probed file.
	 *
	 * At most @ref header_size bytes, and fewer when the file is shorter.
	 * A format reads its magic number out of these; one that needs more than
	 * this to recognize a file is looking at the wrong thing.
	 *
	 * @return span<const byte> The bytes. Empty when the file does not
	 * exist, is empty, or could not be read.
	 */
	REXLIB_API
	span<const byte> get_header() const noexcept;

	/**
	 * @brief Check whether the probed file exists and could be read.
	 *
	 * @return true The file exists and its leading bytes were read.
	 * @return false The file does not exist or could not be read, in which
	 * case @ref get_header is empty.
	 */
	REXLIB_API
	bool exists() const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::string m_path;
	REXLIB_STD_MEMBER_INTERFACE
	std::string m_extension;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<byte> m_header;
	bool m_exists;
};

} // namespace em
} // namespace rexlib
