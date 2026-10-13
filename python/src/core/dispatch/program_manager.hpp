// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/dispatch/program_manager.hpp>

#include <memory>

namespace rexlib
{

using program_manager_class =
	nanobind::class_<program_manager>;

program_manager_class declare_program_manager(nanobind::module_ &m);
void define_program_manager(nanobind::module_ &m);

} // namespace rexlib
