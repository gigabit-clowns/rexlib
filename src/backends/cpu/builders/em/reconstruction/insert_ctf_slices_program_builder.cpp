// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>

#include "ctf_weight.hpp"
#include "slice_insertion_program_builder.hpp"

#include <backends/cpu/dispatch/program_builder_registration_macros.hpp>

namespace rexlib
{
namespace cpu
{

REXLIB_REGISTER_CPU_PROGRAM_BUILDER(
	insert_ctf_slices,
	slice_insertion_program_builder<
		em::insert_ctf_slices_operation,
		ctf_weight
	>
);

} // namespace cpu
} // namespace rexlib
