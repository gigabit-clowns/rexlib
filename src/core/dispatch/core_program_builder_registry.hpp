// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder_registry.hpp>

namespace rexlib
{

/**
 * @brief Access the core library's program builder registry.
 *
 * This is the single registry instance that the program builders bundled in
 * the core shared library auto-register into during static initialization.
 * It is deliberately internal: external modules can not name it, so they can 
 * not auto-register into the core. A plugin owns and drains its own @ref 
 * program_builder_registry instead.
 *
 * @return program_builder_registry& The core registry.
 */
program_builder_registry& get_core_program_builder_registry() noexcept;

} // namespace rexlib
