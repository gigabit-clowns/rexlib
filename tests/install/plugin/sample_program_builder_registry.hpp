// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder_registry.hpp>

namespace sample
{

/**
 * @brief Access the registry of the sample backend's program builders.
 *
 * The builders add themselves to it during static initialization.
 *
 * @return rexlib::program_builder_registry& The registry.
 */
rexlib::program_builder_registry&
get_sample_program_builder_registry() noexcept;

} // namespace sample
