// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_builder_registry.hpp"

#include "program_builder.hpp"
#include "program_service.hpp"

namespace rexlib
{

inline
void program_builder_registry::add(program_builder_factory factory)
{
	if (factory != nullptr)
	{
		m_factories.push_back(factory);
	}
}

inline
void program_builder_registry::register_all(program_service &programs) const
{
	for (const auto factory : m_factories)
	{
		programs.register_builder(factory());
	}
}

} // namespace rexlib
