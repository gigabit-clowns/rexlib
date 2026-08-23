// SPDX-License-Identifier: GPL-3.0-only

#include "core_program_builder_registry.hpp"

namespace rex
{

program_builder_registry& get_core_program_builder_registry() noexcept
{
	static program_builder_registry registry;
	return registry;
}

} // namespace rex
