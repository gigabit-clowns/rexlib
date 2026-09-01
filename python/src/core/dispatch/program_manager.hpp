// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/dispatch/program_manager.hpp>

#include <memory>

namespace rexlib
{

using program_manager_class =
	pybind11::class_<program_manager, std::shared_ptr<program_manager>>;

program_manager_class declare_program_manager(pybind11::module_ &m);
void define_program_manager(pybind11::module_ &m);

} // namespace rexlib
