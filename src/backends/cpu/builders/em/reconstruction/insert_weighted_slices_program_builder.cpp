// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

#include "slice_insertion_program_builder.hpp"
#include "spectrum_weight.hpp"

#include <backends/cpu/dispatch/program_builder_registration_macros.hpp>

namespace rexlib
{
namespace cpu
{

REXLIB_REGISTER_CPU_PROGRAM_BUILDER(
	insert_weighted_slices,
	slice_insertion_program_builder<
		em::insert_weighted_slices_operation,
		spectrum_weight
	>
);

} // namespace cpu
} // namespace rexlib
