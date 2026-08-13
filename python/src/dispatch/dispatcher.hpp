// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/dispatch/dispatcher.hpp>

#include <memory>

namespace xmipp4
{
namespace dispatch
{

using dispatcher_class =
	pybind11::class_<dispatcher, std::shared_ptr<dispatcher>>;

dispatcher_class declare_dispatcher(pybind11::module_ &m);
void define_dispatcher(pybind11::module_ &m);

} // namespace dispatch
} // namespace xmipp4
