// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/creation/linspace_operation.hpp>

#include <backends/cpu/builders/sequence_program_builder.hpp>
#include <backends/cpu/load_store.hpp>

#include <cstddef>

namespace xmipp4
{
namespace cpu
{

namespace
{

template <typename T>
class linspace_kernel
{
public:
	using compute_type = typename element_compute_type<T>::type;

	linspace_kernel(
		compute_type start,
		compute_type step,
		compute_type stop,
		std::size_t stop_index
	) noexcept
		: m_start(start)
		, m_step(step)
		, m_stop(stop)
		, m_stop_index(stop_index)
	{
	}

	void operator()(T *destination, std::size_t index) const noexcept
	{
		// The sample that is meant to land on the stop is written as the
		// stop, so that the far end of the segment is exact whatever the
		// spacing rounds to. The near end is exact already, the step being
		// multiplied by zero there.
		const auto value = (index == m_stop_index)
			? m_stop
			: m_start + numerical_cast<compute_type>(index) * m_step;

		store(destination, value);
	}

private:
	compute_type m_start;
	compute_type m_step;
	compute_type m_stop;
	std::size_t m_stop_index;
};

struct linspace_kernel_factory
{
	template <typename T>
	linspace_kernel<T> operator()(
		const ops::linspace_operation &operation,
		type_list<T> /*output_types*/,
		type_list<> /*input_types*/
	) const
	{
		using compute_type = typename linspace_kernel<T>::compute_type;

		const auto count = operation.get_count();
		const auto endpoint = operation.get_endpoint();
		const auto start = scalar_value_cast<compute_type>(
			operation.get_start()
		);
		const auto stop = scalar_value_cast<compute_type>(
			operation.get_stop()
		);

		const auto inclusive_intervals = count > 1 ? count - 1 : 1;
		const auto exclusive_intervals = count > 0 ? count : 1;
		const auto intervals = endpoint
			? inclusive_intervals
			: exclusive_intervals;
		const auto step =
			(stop - start) / numerical_cast<compute_type>(intervals);

		// Out of range when there is no sample to pin to the stop, so that
		// the single sample of a length one sequence keeps its start.
		const auto stop_index = (endpoint && count > 1) ? count - 1 : count;

		return linspace_kernel<T>(start, step, stop, stop_index);
	}
};

} // anonymous namespace

REX_REGISTER_SEQUENCE_PROGRAM_BUILDER(
	linspace,
	ops::linspace_operation,
	linspace_kernel_factory
);

} // namespace cpu
} // namespace xmipp4
