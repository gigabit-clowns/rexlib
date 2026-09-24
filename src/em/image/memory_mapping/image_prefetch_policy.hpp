// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <em/config.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

/**
 * @brief Widest gap that merging bridges, whatever one region spans.
 *
 * See em/config.hpp, which is where the number comes from and where it may be
 * overridden.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t default_prefetch_gap_cap =
	REXLIB_PREFETCH_GAP_CAP;

/**
 * @brief Bytes a step advises before the regions of the previous one are
 * walked.
 *
 * See em/config.hpp, which is where the number comes from and where it may be
 * overridden.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t default_prefetch_budget =
	REXLIB_PREFETCH_BYTE_BUDGET;

/**
 * @brief What a batch is advised with.
 */
class image_prefetch_policy
{
public:
	/**
	 * @brief Construct a policy from what it settles.
	 *
	 * @param gap_tolerance Widest gap between two stretches that still merges
	 * them.
	 * @param byte_budget Most bytes one step advises, which it exceeds only
	 * where a single region is wider than it.
	 * @param page_size Boundary every stretch is made to start on, which must
	 * not be zero.
	 */
	image_prefetch_policy(
		std::size_t gap_tolerance,
		std::size_t byte_budget,
		std::size_t page_size
	) noexcept;

	image_prefetch_policy(const image_prefetch_policy &other) = default;
	image_prefetch_policy(image_prefetch_policy &&other) noexcept = default;
	~image_prefetch_policy() = default;

	image_prefetch_policy&
	operator=(const image_prefetch_policy &other) = default;
	image_prefetch_policy&
	operator=(image_prefetch_policy &&other) noexcept = default;

	/**
	 * @brief Get the widest gap that still merges two stretches.
	 *
	 * @return std::size_t The gap, in bytes.
	 */
	std::size_t get_gap_tolerance() const noexcept;

	/**
	 * @brief Get the most bytes one step advises.
	 *
	 * @return std::size_t The budget, in bytes.
	 */
	std::size_t get_byte_budget() const noexcept;

	/**
	 * @brief Get the boundary every stretch is made to start on.
	 *
	 * @return std::size_t The page size, in bytes.
	 */
	std::size_t get_page_size() const noexcept;

private:
	std::size_t m_gap_tolerance;
	std::size_t m_byte_budget;
	std::size_t m_page_size;
};

/**
 * @brief Work out the policy a batch of a given region span is advised with.
 *
 * The tolerance scales with the region rather than being flat, so that a gap
 * wider than the data on either side of it is never bridged; it is floored at
 * a page, below which two stretches would be advised as the same page twice,
 * and capped at @ref default_prefetch_gap_cap.
 *
 * @param region_span What one region spans, in bytes.
 * @return image_prefetch_policy The policy, over the page size of the machine.
 */
image_prefetch_policy make_prefetch_policy(std::size_t region_span) noexcept;

} // namespace em
} // namespace rexlib
