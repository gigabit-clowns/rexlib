// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"

#include <memory>
#include <vector>

namespace rexlib
{

class program_builder;
class program_manager;

/**
 * @brief Factory function that creates a fresh program_builder instance.
 */
using program_builder_factory =
	std::unique_ptr<program_builder> (*)();

/**
 * @brief Collects program_builder factories for bulk registration into a
 * program_manager.
 *
 * @note @ref add is not thread-safe. It is meant to run during the owning
 * module's static initialization, before any concurrent access. Reads
 * (@ref register_all) happen strictly afterwards.
 */
class program_builder_registry
{
public:
	REXLIB_API program_builder_registry();
	program_builder_registry(const program_builder_registry &other) = delete;
	program_builder_registry(program_builder_registry &&other) = delete;
	REXLIB_API ~program_builder_registry();

	program_builder_registry&
	operator=(const program_builder_registry &other) = delete;
	program_builder_registry&
	operator=(program_builder_registry &&other) = delete;

	/**
	 * @brief Append a builder factory to the registry.
	 *
	 * @param factory Function creating a fresh builder instance. Null
	 * factories are ignored.
	 */
	REXLIB_API
	void add(program_builder_factory factory);

	/**
	 * @brief Instantiate one builder per registered factory and register them
	 * into a manager.
	 *
	 * @param manager The manager where the builders are registered.
	 */
	REXLIB_API
	void register_all(program_manager &manager) const;

private:
	std::vector<program_builder_factory> m_factories;
};

} // namespace rexlib
