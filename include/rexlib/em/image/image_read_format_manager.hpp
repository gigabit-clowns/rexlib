// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/service_manager.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{

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
	 * @param path Path to the file to open.
	 * @return std::shared_ptr<image_reader> The opened reader, never null.
	 * @throws invalid_operation_error If no registered format recognizes the
	 * file.
	 * @throws image_format_error If the file is malformed or truncated.
	 */
	std::shared_ptr<image_reader> open(const std::string &path) const;

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

/**
 * @brief Get the extents of a file through a format manager.
 *
 * The peer of the @ref image_reader_provider overload, for a caller holding
 * the formats rather than something that serves readers from them. It opens
 * the file every time it is asked, keeping nothing, so a caller asking about
 * one file repeatedly goes through a provider instead.
 *
 * @param formats The formats the file may be opened with.
 * @param path Path to the file.
 * @return std::vector<std::size_t> The extents of the file, slowest axis
 * first.
 * @throws invalid_operation_error If no registered format recognizes the
 * file.
 * @throws image_format_error If the file is malformed or truncated.
 */
REXLIB_API
std::vector<std::size_t> query_extents(
	const image_read_format_manager &formats,
	const std::string &path
);

/**
 * @brief Get the extents of one image or volume of a file through a format
 * manager.
 *
 * The trailing @ref image_reader::get_core_rank extents of the file, so the
 * axes it stacks along are left out. That is the shape of a single image or
 * volume, which is what the destination of a batch carries beside its leading
 * extent.
 *
 * @param formats The formats the file may be opened with.
 * @param path Path to the file.
 * @return std::vector<std::size_t> The extents of one image or volume.
 * @throws invalid_operation_error If no registered format recognizes the
 * file.
 * @throws image_format_error If the file is malformed or truncated.
 */
REXLIB_API
std::vector<std::size_t> query_core_extents(
	const image_read_format_manager &formats,
	const std::string &path
);

} // namespace em
} // namespace rexlib
