// SPDX-License-Identifier: GPL-3.0-only

#include "cpu_program_builder_registry.hpp"

namespace rexlib
{
namespace cpu
{

program_builder_registry& get_cpu_program_builder_registry() noexcept
{
	static program_builder_registry registry;
	return registry;
}

} // namespace cpu
} // namespace rexlib
