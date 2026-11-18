// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include <vector>

namespace rexlib
{

class program_builder;
class program_service;

/**
 * @brief Factory function that creates a fresh program_builder instance.
 */
using program_builder_factory =
	std::unique_ptr<program_builder> (*)();

/**
 * @brief Collects program_builder factories for bulk registration into a
 * program_service.
 *
 * Its members are defined in this header, so a shared object that fills it
 * as it loads runs no code of the library.
 *
 * @note @ref add is not thread-safe. It is meant to run during the owning
 * module's static initialization, before any concurrent access. Reads
 * (@ref register_all) happen strictly afterwards.
 */
class program_builder_registry
{
public:
	program_builder_registry() = default;
	program_builder_registry(const program_builder_registry &other) = delete;
	program_builder_registry(program_builder_registry &&other) = delete;
	~program_builder_registry() = default;

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
	void add(program_builder_factory factory);

	/**
	 * @brief Instantiate one builder per registered factory and register them
	 * into a program service.
	 *
	 * @param programs The program service where the builders are registered.
	 */
	void register_all(program_service &programs) const;

private:
	std::vector<program_builder_factory> m_factories;
};

} // namespace rexlib

#include "program_builder_registry.inl"
