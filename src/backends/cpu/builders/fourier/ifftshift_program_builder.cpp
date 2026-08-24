// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/ifftshift_operation.hpp>

#include <backends/cpu/builders/roll_program_builder.hpp>

#include <cstddef>

namespace rex
{
namespace cpu
{

namespace
{

// The inverse of fftshift: undoes a forward shift of n/2 by shifting
// backward by the same amount, i.e. forward by n - n/2. The two formulas
// agree for even extents and differ by one for odd ones, matching
// fftshift/ifftshift only differing on axes of odd extent.
struct ifftshift_shift_policy
{
	static std::size_t shift_amount(std::size_t extent) noexcept
	{
		return extent - extent / 2;
	}
};

} // anonymous namespace

REXLIB_REGISTER_ROLL_PROGRAM_BUILDER(
	ifftshift,
	ops::ifftshift_operation,
	ifftshift_shift_policy
);

} // namespace cpu
} // namespace rex
