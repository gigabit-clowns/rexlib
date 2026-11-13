// SPDX-License-Identifier: GPL-3.0-only

#include "sample_program_builder_registry.hpp"

namespace sample
{

rexlib::program_builder_registry&
get_sample_program_builder_registry() noexcept
{
	static rexlib::program_builder_registry registry;
	return registry;
}

} // namespace sample
