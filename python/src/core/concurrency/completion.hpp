// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/concurrency/completion.hpp>

#include <memory>

namespace rexlib
{

using completion_class =
	pybind11::class_<completion, std::shared_ptr<completion>>;

completion_class declare_completion(pybind11::module_ &m);
void define_completion(completion_class &c);

} // namespace rexlib
