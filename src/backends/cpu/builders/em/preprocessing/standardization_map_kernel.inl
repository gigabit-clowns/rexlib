// SPDX-License-Identifier: GPL-3.0-only

#include "standardization_map_kernel.hpp"

#include <backends/cpu/kernels/standard_normal.hpp>
#include <backends/cpu/load_store.hpp>
#include <backends/cpu/loops/strided_pointer_iterator.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace rexlib
{
namespace cpu
{

inline
standardization_map_kernel::standardization_map_kernel(
	double mean,
	double scale,
	double lower_bound,
	double upper_bound,
	std::uint64_t seed,
	std::uint64_t counter
) noexcept
	: m_mean(mean)
	, m_scale(scale)
	, m_lower_bound(lower_bound)
	, m_upper_bound(upper_bound)
	, m_seed(seed)
	, m_counter(counter)
{
}

template <
	typename Out,
	typename In,
	typename OutStride,
	typename InStride,
	typename IndexRun
>
inline
void standardization_map_kernel::operator()(
	const std::tuple<Out*, const In*> &pointers,
	const std::tuple<OutStride, InStride> &strides,
	std::size_t count,
	const IndexRun &index
) const noexcept
{
	using compute_type = typename element_compute_type<In>::type;

	const auto mean = static_cast<compute_type>(m_mean);
	const auto scale = static_cast<compute_type>(m_scale);
	const auto lower_bound = static_cast<compute_type>(m_lower_bound);
	const auto upper_bound = static_cast<compute_type>(m_upper_bound);

	auto result = make_strided_pointer_iterator(
		std::get<0>(pointers),
		std::get<0>(strides)
	);
	auto value = make_strided_pointer_iterator(
		std::get<1>(pointers),
		std::get<1>(strides)
	);

	using flag_type = typename std::conditional<
		sizeof(compute_type) == sizeof(std::uint64_t),
		std::uint64_t,
		std::uint32_t
	>::type;

	flag_type replaced = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto n = static_cast<std::ptrdiff_t>(i);
		const auto standardized =
			(static_cast<compute_type>(load(&value[n])) - mean) * scale;
		store(&result[n], standardized);
		replaced |= static_cast<flag_type>(
			!is_kept(standardized, lower_bound, upper_bound)
		);
	}

	if (replaced == 0)
	{
		return;
	}

	bool has_pair = false;
	std::uint64_t pair_block = 0;
	std::array<double, 2> pair = {{ 0.0, 0.0 }};
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto n = static_cast<std::ptrdiff_t>(i);
		const auto standardized =
			(static_cast<compute_type>(load(&value[n])) - mean) * scale;
		if (is_kept(standardized, lower_bound, upper_bound))
		{
			continue;
		}

		const std::uint64_t counter =
			m_counter + index.advanced(i).get_index();
		if (!has_pair || counter / 2 != pair_block)
		{
			pair_block = counter / 2;
			pair = standard_normal_pair(m_seed, pair_block);
			has_pair = true;
		}
		store(&result[n], static_cast<compute_type>(pair[counter % 2]));
	}
}

template <typename T>
inline
bool standardization_map_kernel::is_kept(
	T standardized,
	T lower_bound,
	T upper_bound
) const noexcept
{
	return
		(standardized >= lower_bound) &
		(standardized <= upper_bound) &
		(std::abs(standardized) <= std::numeric_limits<T>::max());
}

} // namespace cpu
} // namespace rexlib
