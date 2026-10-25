// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_builder_registration.hpp"

namespace rexlib
{

template <typename Builder, typename Registry>
inline
program_builder_registration<Builder, Registry>::program_builder_registration(
	Registry &registry
)
{
	registry.add(&create_builder);
}

template <typename Builder, typename Registry>
inline
std::unique_ptr<program_builder>
program_builder_registration<Builder, Registry>::create_builder()
{
	return std::make_unique<Builder>();
}

} // namespace rexlib
