// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/ops/fourier/fourier_normalization.hpp>

namespace rexlib
{
namespace ops
{

using fourier_normalization_class = nanobind::enum_<fourier_normalization>;

fourier_normalization_class declare_fourier_normalization(
	nanobind::module_ &m
);
void define_fourier_normalization(fourier_normalization_class &c);

} // namespace ops
} // namespace rexlib
