// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/reduction/mean_operation.hpp>

#include <xmipp4/core/numerical/numerical_cast.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/fold_reduction_kernel.hpp>
#include <backends/cpu/load_store.hpp>

#include <cstddef>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct sum_fold
{
	template <typename T>
	T operator()(const T &accumulator, const T &value) const noexcept
	{
		return accumulator + value;
	}

	template <typename T>
	static T identity() noexcept
	{
		return T(0);
	}
};

/**
 * @brief Total the elements, then divide by how many there were.
 *
 * An average is a sum with a post processing, so it inherits the sum and
 * replaces only the step that writes the answer out. The division happens
 * once per output element rather than once per element folded, which is
 * what keeps it off the hot loop.
 *
 * The accumulator is the inexact counterpart of the operand type, which the
 * operation's own rule already asks for, so an integer array averages in
 * float64 and no separate decision about precision is made here.
 */
class mean_kernel : public fold_reduction_kernel<sum_fold>
{
public:
	template <typename U, typename Accumulator>
	void finalize(
		U *result,
		const Accumulator &total,
		std::size_t count
	) const noexcept
	{
		// An average of nothing is a division of zero by zero, which the
		// inexact result type answers with a not-a-number, as NumPy does.
		store(result, total / numerical_cast<Accumulator>(count));
	}
};

} // anonymous namespace

REX_REGISTER_REDUCTION_PROGRAM_BUILDER(
	mean,
	ops::mean_operation,
	default_kernel_factory<mean_kernel>
);

} // namespace cpu
} // namespace xmipp4
