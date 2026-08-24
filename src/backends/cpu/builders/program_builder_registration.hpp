// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/program_builder_registry.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

/**
 * @brief Instantiate and auto-register a CPU program builder.
 *
 * The one spelling of a registration object every builder family shares.
 * A family wraps this in a macro of its own naming the builder template, so
 * that an operation's translation unit says which family it belongs to and
 * nothing about the registry.
 *
 * @param name Identifier of the registration object.
 * @param ... The builder type. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REXLIB_REGISTER_CPU_PROGRAM_BUILDER(name, ...) \
	static const ::rexlib::program_builder_registration<__VA_ARGS__> \
	name##_program_builder_registration( \
		::rexlib::get_core_program_builder_registry() \
	)
