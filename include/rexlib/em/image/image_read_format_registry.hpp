// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>
#include <vector>

namespace rexlib
{
namespace em
{

class image_read_format;
class image_read_format_manager;

/**
 * @brief Factory function that creates a fresh image_read_format instance.
 */
using image_read_format_factory =
	std::unique_ptr<image_read_format> (*)();

/**
 * @brief Collects image read format factories for bulk registration.
 *
 * A module owns its own registry and drains it into a manager, rather than
 * adding to a registry belonging to somebody else.
 *
 * @note @ref add is not thread-safe. It is meant to run during the owning
 * module's static initialization, before any concurrent access. Reads
 * (@ref register_all) happen strictly afterwards.
 */
class image_read_format_registry
{
public:
	using format_type = image_read_format;

	REXLIB_API image_read_format_registry();
	image_read_format_registry(
		const image_read_format_registry &other
	) = delete;
	image_read_format_registry(image_read_format_registry &&other) = delete;
	REXLIB_API ~image_read_format_registry();

	image_read_format_registry&
	operator=(const image_read_format_registry &other) = delete;
	image_read_format_registry&
	operator=(image_read_format_registry &&other) = delete;

	/**
	 * @brief Append a format factory to the registry.
	 *
	 * @param factory Function creating a fresh format instance. Null
	 * factories are ignored.
	 */
	REXLIB_API
	void add(image_read_format_factory factory);

	/**
	 * @brief Instantiate one format per registered factory and register them
	 * into a manager.
	 *
	 * @param manager The manager where the formats are registered.
	 */
	REXLIB_API
	void register_all(image_read_format_manager &manager) const;

private:
	std::vector<image_read_format_factory> m_factories;
};

} // namespace em
} // namespace rexlib
