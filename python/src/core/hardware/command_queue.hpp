// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/command_queue.hpp>

#include <memory>

namespace rexlib
{

using command_queue_class = nanobind::class_<command_queue>;

command_queue_class declare_command_queue(nanobind::module_ &m);
void define_command_queue(command_queue_class &c);

} // namespace rexlib
