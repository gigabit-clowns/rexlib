// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/service_manager.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_open_options;
class image_probe;
class image_read_format;

/**
 * @brief Centralizes all known image formats that can be read.
 *
 * Reached through @ref service_catalog::get_service_manager. The formats
 * bundled with the library are added by @ref register_builtin_backends, and
 * a plugin adds its own from @ref plugin::register_at.
 *
 * Writing is served by @ref image_write_format_manager, which is a separate
 * service, so a program that only reads never constructs it.
 */
class REXLIB_API image_read_format_manager final
	: public service_manager
{
public:
	image_read_format_manager() noexcept;
	image_read_format_manager(
		const image_read_format_manager &other
	) = delete;
	image_read_format_manager(image_read_format_manager &&other) = delete;
	~image_read_format_manager() override;

	image_read_format_manager&
	operator=(const image_read_format_manager &other) = delete;
	image_read_format_manager&
	operator=(image_read_format_manager &&other) = delete;

	void register_builtin_backends() override;

	/**
	 * @brief Register a new image read format.
	 *
	 * @param format The format to be registered.
	 * @return true The format was successfully registered.
	 * @return false The format was null.
	 */
	bool register_format(std::unique_ptr<image_read_format> format);

	/**
	 * @brief Open a file for reading with the most suitable format.
	 *
	 * Reads the head of the file once and shows it to every registered
	 * format, then opens the file with whichever reported the highest
	 * suitability.
	 *
	 * @param path Path to the file holding the file.
	 * @param options Which image of the file to open and how it will be
	 * walked.
	 * @return std::unique_ptr<image_reader> The opened reader, never null.
	 * @throws invalid_operation_error If no registered format recognizes the
	 * file.
	 * @throws image_format_error If the file is malformed or truncated.
	 */
	std::unique_ptr<image_reader> open(
		const std::string &path,
		const image_open_options &options
	) const;

	/**
	 * @brief Get the format that would open a file, without opening it.
	 *
	 * Answers which format claims a file, and whether any does at all,
	 * without the cost or the failure modes of opening it.
	 *
	 * @param probe The file under consideration.
	 * @return const image_read_format* The most suitable format, or nullptr
	 * when none recognizes @p probe.
	 */
	const image_read_format*
	get_most_suitable_format(const image_probe &probe) const;

private:
	class implementation;
	REXLIB_STD_MEMBER_INTERFACE
	std::unique_ptr<implementation> m_implementation;

	implementation& create_if_null();
	const implementation& get_implementation() const noexcept;
};

} // namespace em
} // namespace rexlib
