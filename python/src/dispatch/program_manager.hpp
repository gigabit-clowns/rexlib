// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/dispatch/program_manager.hpp>

#include <memory>

namespace xmipp4
{
namespace dispatch
{

using program_manager_class =
	pybind11::class_<program_manager, std::shared_ptr<program_manager>>;

program_manager_class declare_program_manager(pybind11::module_ &m);
void define_program_manager(pybind11::module_ &m);

} // namespace dispatch
} // namespace xmipp4
