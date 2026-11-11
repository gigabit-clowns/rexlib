// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/layouts/named_core_frame.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Split the axes of the outputs of an operation into their core and
 * their batch axes.
 *
 * The core axes are the ones the operation names, or its trailing ones,
 * as its shape rule places them.
 *
 * @tparam Op The operation. Its parameters name the core axes through
 * get_axes() when its shape rule names them. When its shape rule places
 * them last, the core of its first output has a fixed number of axes.
 * @param operation The operation.
 * @param output_descriptors The output operand descriptors. At least one.
 * @return named_core_frame The frame of the first output.
 */
template <typename Op>
named_core_frame make_output_core_frame(
	const Op &operation,
	span<const operand_descriptor> output_descriptors
);

} // namespace cpu
} // namespace rexlib

#include "output_core_frame.inl"
