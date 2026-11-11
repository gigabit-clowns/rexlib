// SPDX-License-Identifier: GPL-3.0-only

#include "fourier_conversion.hpp"

#include <backends/cpu/numerical/load_store.hpp>
#include <backends/cpu/loops/elementwise_loop.hpp>

namespace rexlib
{
namespace cpu
{

namespace detail
{

struct fourier_conversion_kernel
{
	template <typename T, typename Q>
	void operator()(T *destination, const Q *source) const noexcept
	{
		cast(destination, source);
	}
};

} // namespace detail

template <typename T, typename Q>
inline
void run_fourier_conversion(
	const fourier_layout_plan &plan,
	T *output,
	const Q *input,
	const loop_schedule &schedule
)
{
	run_elementwise_loop(
		detail::fourier_conversion_kernel(),
		plan.get_conversion_layout(),
		schedule,
		output,
		input
	);
}

} // namespace cpu
} // namespace rexlib
