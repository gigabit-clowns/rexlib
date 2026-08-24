// SPDX-License-Identifier: GPL-3.0-only

#include "reduction_kernel_traits.hpp"

#include "../config.hpp"

#include <rex/core/platform/constexpr.hpp>

#include <utility>

namespace rexlib
{
namespace cpu
{

template <>
struct accumulator_footprint<type_list<>>
	: std::integral_constant<std::size_t, 0>
{
};

template <typename Head, typename... Tail>
struct accumulator_footprint<type_list<Head, Tail...>>
	: std::integral_constant<
		std::size_t,
		sizeof(Head) + accumulator_footprint<type_list<Tail...>>::value
	>
{
};

namespace detail
{

// How much of a tile the accumulators may take, and how long the tile may
// be as a result. See config.hpp for what the defaults are worth and why.
REXLIB_CONST_CONSTEXPR std::size_t reduction_tile_budget =
	REXLIB_REDUCTION_TILE_BUDGET;
REXLIB_CONST_CONSTEXPR std::size_t minimum_reduction_tile =
	REXLIB_MINIMUM_REDUCTION_TILE_SIZE;
REXLIB_CONST_CONSTEXPR std::size_t maximum_reduction_tile =
	REXLIB_MAXIMUM_REDUCTION_TILE_SIZE;

// The same three for the lanes a run is dealt out over.
REXLIB_CONST_CONSTEXPR std::size_t reduction_fold_lane_budget =
	REXLIB_REDUCTION_FOLD_LANE_BUDGET;
REXLIB_CONST_CONSTEXPR std::size_t minimum_reduction_fold_lanes =
	REXLIB_MINIMUM_REDUCTION_FOLD_LANES;
REXLIB_CONST_CONSTEXPR std::size_t maximum_reduction_fold_lanes =
	REXLIB_MAXIMUM_REDUCTION_FOLD_LANES;

REXLIB_INLINE_CONSTEXPR
std::size_t clamp_reduction_tile(std::size_t count) noexcept
{
	if (count > maximum_reduction_tile)
	{
		return maximum_reduction_tile;
	}

	if (count < minimum_reduction_tile)
	{
		return minimum_reduction_tile;
	}

	return count;
}

// The same three for the accumulators one strip block holds, and the input
// one pass over the blocks of a strip may span.
REXLIB_CONST_CONSTEXPR std::size_t reduction_strip_block_budget =
	REXLIB_REDUCTION_STRIP_BLOCK_BUDGET;
REXLIB_CONST_CONSTEXPR std::size_t minimum_reduction_strip_block =
	REXLIB_MINIMUM_REDUCTION_STRIP_BLOCK;
REXLIB_CONST_CONSTEXPR std::size_t maximum_reduction_strip_block =
	REXLIB_MAXIMUM_REDUCTION_STRIP_BLOCK;
REXLIB_CONST_CONSTEXPR std::size_t reduction_strip_pass_budget =
	REXLIB_REDUCTION_STRIP_PASS_BUDGET;
REXLIB_CONST_CONSTEXPR std::size_t minimum_reduction_strip_block_run =
	REXLIB_MINIMUM_REDUCTION_STRIP_BLOCK_RUN;

REXLIB_INLINE_CONSTEXPR
std::size_t clamp_reduction_strip_block(std::size_t count) noexcept
{
	if (count > maximum_reduction_strip_block)
	{
		return maximum_reduction_strip_block;
	}

	if (count < minimum_reduction_strip_block)
	{
		return minimum_reduction_strip_block;
	}

	return count;
}

REXLIB_INLINE_CONSTEXPR
std::size_t clamp_reduction_fold_lanes(std::size_t count) noexcept
{
	if (count > maximum_reduction_fold_lanes)
	{
		return maximum_reduction_fold_lanes;
	}

	if (count < minimum_reduction_fold_lanes)
	{
		return minimum_reduction_fold_lanes;
	}

	return count;
}

/**
 * @brief Detect a kernel's optional identity member.
 */
template <typename Kernel, typename... Accumulators>
class identity_detector
{
	template <typename K>
	static auto probe(int) -> decltype(
		std::declval<const K&>().identity(
			std::declval<Accumulators&>()...
		),
		std::true_type()
	);

	template <typename K>
	static std::false_type probe(...);

public:
	using type = decltype(probe<Kernel>(0));
};

template <typename... Accumulators, std::size_t Size>
struct accumulator_tiles<type_list<Accumulators...>, Size>
{
	using type = std::tuple<std::array<Accumulators, Size>...>;
};

} // namespace detail

template <typename Accumulators>
struct reduction_tile_size
	: std::integral_constant<
		std::size_t,
		detail::clamp_reduction_tile(
			detail::reduction_tile_budget /
			accumulator_footprint<Accumulators>::value
		)
	>
{
};

template <typename Accumulators>
struct reduction_fold_lane_count
	: std::integral_constant<
		std::size_t,
		detail::clamp_reduction_fold_lanes(
			detail::reduction_fold_lane_budget /
			accumulator_footprint<Accumulators>::value
		)
	>
{
};

template <typename Accumulators>
struct reduction_strip_block_size
	: std::integral_constant<
		std::size_t,
		detail::clamp_reduction_strip_block(
			detail::reduction_strip_block_budget /
			accumulator_footprint<Accumulators>::value
		)
	>
{
};

template <typename Kernel, typename... Accumulators>
struct has_reduction_identity<Kernel, type_list<Accumulators...>>
	: detail::identity_detector<Kernel, Accumulators...>::type
{
};

// Saying nothing is saying no: the specialization is chosen when the member
// exists, and what it says is then read from it rather than assumed. Folding
// the two questions into one substitution would answer them both with the
// same yes, and a kernel that declared it false would be reassociated anyway.
template <typename Kernel, typename>
struct has_reassociable_fold
	: std::false_type
{
};

template <typename Kernel>
struct has_reassociable_fold<
	Kernel,
	std::void_t<decltype(Kernel::reassociable_fold)>
>
	: std::integral_constant<bool, Kernel::reassociable_fold>
{
};

} // namespace cpu
} // namespace rexlib
