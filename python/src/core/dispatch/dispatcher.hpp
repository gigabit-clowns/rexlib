// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/dispatch/dispatcher.hpp>

#include <memory>

namespace rexlib
{

using dispatcher_class =
	nanobind::class_<dispatcher>;

dispatcher_class declare_dispatcher(nanobind::module_ &m);
void define_dispatcher(nanobind::module_ &m);

} // namespace rexlib
