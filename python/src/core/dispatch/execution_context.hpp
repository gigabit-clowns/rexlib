// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/dispatch/execution_context.hpp>

#include <memory>

namespace rexlib
{

using execution_context_class = pybind11::class_<execution_context>;

execution_context_class declare_execution_context(pybind11::module_ &m);
void define_execution_context(execution_context_class &c);

} // namespace rexlib
