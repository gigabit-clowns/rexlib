// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/program_builder_registry.hpp>

#include <rexlib/core/dispatch/program_builder.hpp>
#include <rexlib/core/dispatch/program_manager.hpp>

namespace rexlib
{

program_builder_registry::program_builder_registry() = default;
program_builder_registry::~program_builder_registry() = default;

void program_builder_registry::add(program_builder_factory factory)
{
	if (factory != nullptr)
	{
		m_factories.push_back(factory);
	}
}

void program_builder_registry::register_all(program_manager &manager) const
{
	for (const auto factory : m_factories)
	{
		manager.register_builder(factory());
	}
}

} // namespace rexlib
