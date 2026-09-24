// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_builder_registry.hpp"

#include <memory>

namespace rexlib
{

class program_builder;

/**
 * @brief Appends a factory for @p Builder to a registry upon construction.
 *
 * Declare one namespace-scope object of this type per builder translation
 * unit so the builder auto-registers during static initialization.
 *
 * @tparam Builder The concrete program_builder type. Must be default
 * constructible.
 */
template <typename Builder>
class program_builder_registration
{
public:
	/**
	 * @brief Append a factory for @p Builder to a registry.
	 *
	 * @param registry The registry to append to.
	 */
	explicit program_builder_registration(program_builder_registry &registry);

private:
	static std::unique_ptr<program_builder> create_builder();
};

} // namespace rexlib

#include "program_builder_registration.inl"
