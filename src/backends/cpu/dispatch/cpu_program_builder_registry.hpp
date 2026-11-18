// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder_registry.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Access the registry of the CPU backend's program builders.
 *
 * The CPU's program builders add themselves to it during static
 * initialization, through @ref REXLIB_REGISTER_CPU_PROGRAM_BUILDER, and
 * @ref cpu_plugin registers them at a catalog's program_service.
 *
 * @return program_builder_registry& The registry.
 */
program_builder_registry& get_cpu_program_builder_registry() noexcept;

} // namespace cpu
} // namespace rexlib
