// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/service_manager.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_metadata;
class image_probe;
class image_write_format;

/**
 * @brief Centralizes all known image formats that can be written.
 *
 * The counterpart of @ref image_read_format_manager, kept as a service of
 * its own so that the reading and the writing sides are registered, looked
 * up and reasoned about separately.
 */
class REXLIB_API image_write_format_manager final
	: public service_manager
{
public:
	image_write_format_manager() noexcept;
	image_write_format_manager(
		const image_write_format_manager &other
	) = delete;
	image_write_format_manager(image_write_format_manager &&other) = delete;
	~image_write_format_manager() override;

	image_write_format_manager&
	operator=(const image_write_format_manager &other) = delete;
	image_write_format_manager&
	operator=(image_write_format_manager &&other) = delete;

	void register_builtin_backends() override;

	/**
	 * @brief Register a new image write format.
	 *
	 * @param format The format to be registered.
	 * @return true The format was successfully registered.
	 * @return false The format was null.
	 */
	bool register_format(std::unique_ptr<image_write_format> format);

	/**
	 * @brief Create a file with the most suitable format.
	 *
	 * The file named by @p path usually does not exist yet, so the choice
	 * normally rests on its extension. Any file already there is replaced.
	 *
	 * @param path Path to the file to create.
	 * @param extents Extents of the file to create, slowest axis first.
	 * @param data_type Data type of its elements.
	 * @param metadata How its samples map onto physical space.
	 * @return std::unique_ptr<image_writer> The opened writer, never null.
	 * @throws invalid_operation_error If no registered format recognizes the
	 * file, or if the chosen one can not represent the requested file.
	 * @throws image_format_error If the file could not be created.
	 */
	std::unique_ptr<image_writer> open(
		const std::string &path,
		span<const std::size_t> extents,
		numerical_type data_type,
		const image_metadata &metadata
	) const;

	/**
	 * @brief Get the format that would create a file, without creating it.
	 *
	 * @param probe The file under consideration.
	 * @return const image_write_format* The most suitable format, or nullptr
	 * when none recognizes @p probe.
	 */
	const image_write_format*
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
