// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/dispatch/program_service.hpp>

#include <memory>

namespace rexlib
{

using program_service_class =
	nanobind::class_<program_service>;

program_service_class declare_program_service(nanobind::module_ &m);
void define_program_service(nanobind::module_ &m);

} // namespace rexlib
