// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/command_queue.hpp>

#include <memory>

namespace rexlib
{

using command_queue_class = pybind11::class_<command_queue, std::shared_ptr<command_queue>>;

command_queue_class declare_command_queue(pybind11::module_ &m);
void define_command_queue(command_queue_class &c);

} // namespace rexlib
