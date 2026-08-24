// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/reduction/argmin_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/extremum_locator_kernel.hpp>
#include <backends/cpu/kernels/ordering.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

/**
 * @brief Locate the smallest element along the reduced axes.
 *
 * The same ordering amin takes the smallest element by, asked which of two
 * elements won rather than for the element itself: a not-a-number wins, and
 * a tie keeps the first place it was seen.
 */
struct minimum_order
{
	template <typename T>
	bool operator()(const T &candidate, const T &best) const noexcept
	{
		return precedes(candidate, best);
	}
};

} // anonymous namespace

REXLIB_REGISTER_INDEXED_REDUCTION_PROGRAM_BUILDER(
	argmin,
	ops::argmin_operation,
	default_kernel_factory<extremum_locator_kernel<minimum_order>>
);

} // namespace cpu
} // namespace rexlib
