// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/fftshift_operation.hpp>

#include <backends/cpu/builders/roll_program_builder.hpp>

#include <cstddef>

namespace rex
{
namespace cpu
{

namespace
{

// Moves the zero frequency (index 0) to the middle of the spectrum, at
// index n/2, so an axis of extent n is shifted forward by that amount.
struct fftshift_shift_policy
{
	static std::size_t shift_amount(std::size_t extent) noexcept
	{
		return extent / 2;
	}
};

} // anonymous namespace

REXLIB_REGISTER_ROLL_PROGRAM_BUILDER(
	fftshift,
	ops::fftshift_operation,
	fftshift_shift_policy
);

} // namespace cpu
} // namespace rex
