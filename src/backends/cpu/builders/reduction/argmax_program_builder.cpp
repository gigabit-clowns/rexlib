// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/reduction/argmax_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/extremum_locator_kernel.hpp>
#include <backends/cpu/kernels/ordering.hpp>

namespace xmipp4
{
namespace cpu
{

namespace
{

/**
 * @brief Locate the largest element along the reduced axes.
 *
 * The same ordering amax takes the largest element by, asked which of two
 * elements won rather than for the element itself: a not-a-number wins, and
 * a tie keeps the first place it was seen.
 */
struct maximum_order
{
	template <typename T>
	bool operator()(const T &candidate, const T &best) const noexcept
	{
		return exceeds(candidate, best);
	}
};

} // anonymous namespace

REX_REGISTER_INDEXED_REDUCTION_PROGRAM_BUILDER(
	argmax,
	ops::argmax_operation,
	default_kernel_factory<extremum_locator_kernel<maximum_order>>
);

} // namespace cpu
} // namespace xmipp4
