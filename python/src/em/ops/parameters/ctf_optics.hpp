// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/em/ops/parameters/ctf_optics.hpp>

namespace rexlib
{
namespace em
{

using ctf_optics_class = nanobind::class_<ctf_optics>;

ctf_optics_class declare_ctf_optics(nanobind::module_ &m);
void define_ctf_optics(ctf_optics_class &c);

} // namespace em
} // namespace rexlib
