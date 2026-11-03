// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>

#include "slice_insertion_program_builder.hpp"
#include "unit_weight.hpp"

#include <backends/cpu/builders/program_builder_registration_macros.hpp>

namespace rexlib
{
namespace cpu
{

REXLIB_REGISTER_CPU_PROGRAM_BUILDER(
	insert_slices,
	slice_insertion_program_builder<em::insert_slices_operation, unit_weight>
);

} // namespace cpu
} // namespace rexlib
