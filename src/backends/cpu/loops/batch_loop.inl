// SPDX-License-Identifier: GPL-3.0-only

#include "batch_loop.hpp"

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>

namespace rexlib
{
namespace cpu
{

template <typename Visitor, typename Indexing, typename... Pointers>
inline
void run_batch_loop(
	const Visitor &visitor,
	const joint_layout &batch_layout,
	Indexing indexing,
	const loop_schedule &schedule,
	std::size_t element_cost,
	std::size_t inner_count,
	Pointers... pointers
)
{
	const auto inside = inner_count > 0 && should_split_inner_space(
		batch_layout.compute_element_count(),
		inner_count,
		schedule.get_concurrency()
	);
	const auto outer = inside
		? loop_schedule()
		: schedule.with_grain(grain_for_cost(element_cost));
	const auto inner = inside ? schedule : loop_schedule();

	run_indexed_elementwise_loop(
		[&visitor, &inner] (auto... arguments)
		{
			visitor(inner, arguments...);
		},
		batch_layout,
		indexing,
		outer,
		pointers...
	);
}

} // namespace cpu
} // namespace rexlib
