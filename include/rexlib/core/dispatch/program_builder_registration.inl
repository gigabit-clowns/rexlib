// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_builder_registration.hpp"

namespace rexlib
{

template <typename Builder>
inline
program_builder_registration<Builder>::program_builder_registration(
	program_builder_registry &registry
)
{
	registry.add(&create_builder);
}

template <typename Builder>
inline
std::unique_ptr<program_builder>
program_builder_registration<Builder>::create_builder()
{
	return std::make_unique<Builder>();
}

} // namespace rexlib
