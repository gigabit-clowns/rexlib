// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/core/system/page_prefetch.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class image_transfer_plan;

namespace mrc
{

class mrc_geometry;

/**
 * @brief Widest gap that merging bridges, whatever one region spans.
 *
 * The readahead a kernel does of its own accord is about this wide, so a gap
 * wider than it is left for the fault that reaches it.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t default_prefetch_gap_cap =
	128 * 1024;

/**
 * @brief Bytes a step advises before the regions of the previous one are
 * walked.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t default_prefetch_budget =
	64 * 1024 * 1024;

/**
 * @brief What a batch is advised with.
 */
struct mrc_prefetch_policy
{
	/**
	 * @brief Widest gap between two stretches that still merges them.
	 */
	std::size_t gap_tolerance;

	/**
	 * @brief Most bytes one step advises, which it exceeds only where a
	 * single region is wider than it.
	 */
	std::size_t byte_budget;

	/**
	 * @brief Boundary every stretch is made to start on, which must not be
	 * zero.
	 */
	std::size_t page_size;
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
 * @return mrc_prefetch_policy The policy, over the page size of the machine.
 */
mrc_prefetch_policy make_prefetch_policy(std::size_t region_span) noexcept;

/**
 * @brief Work out what one region of a batch spans, in bytes.
 *
 * The stretch from the first byte of a region to its last, gaps included,
 * which is what a region occupies when it is not contiguous. Every region of
 * a batch shares the extents, so all of them span the same.
 *
 * @param regions The regions to be moved.
 * @param geometry The shape of the file they address.
 * @return std::size_t The span in bytes, zero for a region of no elements.
 */
std::size_t compute_region_span(
	const image_transfer_plan &regions,
	const mrc_geometry &geometry
) noexcept;

/**
 * @brief The stretches of a file a batch reaches, grouped into steps.
 *
 * The regions of a batch are advised as the stretches they actually touch
 * rather than as the one span that covers them, so that a batch drawn at
 * random out of a large file asks for what it reads and not for the file.
 * Stretches that overlap or that lie within the tolerance of each other are
 * merged, which collapses a batch reading consecutive regions back into the
 * single stretch it is.
 *
 * Every stretch is made to start on a page boundary and to end within the
 * mapping here, once, so that what @ref prefetch_pages is handed is already
 * what it asks for.
 *
 * The merged stretches are then grouped into steps of a bounded number of
 * bytes, so that a reader advises one step ahead of the one it is walking
 * instead of the whole batch at once. A batch that fits in the budget is one
 * step, which is what a batch of small scattered regions wants: all of it is
 * advised before any of it is read, and every latency overlaps. A step always
 * holds at least one region, so a region wider than the budget is advised
 * whole.
 *
 * The file offsets this is built from must be ascending, which is how
 * @ref mrc_region_offsets holds them.
 */
class mrc_region_prefetch_plan
{
public:
	/**
	 * @brief Work out the stretches of a batch and the steps they fall in.
	 *
	 * @param regions The regions to be moved.
	 * @param geometry The shape of the file they address.
	 * @param file_offsets Where each region starts in the file, in elements,
	 * ascending.
	 * @param mapped_size How many bytes of the file are mapped, which no
	 * stretch reaches past.
	 * @param policy What the batch is advised with.
	 */
	mrc_region_prefetch_plan(
		const image_transfer_plan &regions,
		const mrc_geometry &geometry,
		span<const std::ptrdiff_t> file_offsets,
		std::size_t mapped_size,
		const mrc_prefetch_policy &policy
	);

	mrc_region_prefetch_plan(const mrc_region_prefetch_plan &other) = default;
	mrc_region_prefetch_plan(
		mrc_region_prefetch_plan &&other
	) noexcept = default;
	~mrc_region_prefetch_plan() = default;

	mrc_region_prefetch_plan&
	operator=(const mrc_region_prefetch_plan &other) = default;
	mrc_region_prefetch_plan&
	operator=(mrc_region_prefetch_plan &&other) noexcept = default;

	/**
	 * @brief Get how many steps the batch is walked in.
	 *
	 * @return std::size_t The number of steps, zero for a batch that reaches
	 * nothing.
	 */
	std::size_t get_step_count() const noexcept;

	/**
	 * @brief Get every stretch the batch reaches.
	 *
	 * @return span<const memory_range> The stretches, ascending and disjoint,
	 * as byte offsets from the start of the mapping.
	 */
	span<const memory_range> get_ranges() const noexcept;

	/**
	 * @brief Get the stretches one step advises.
	 *
	 * @param step Index of the step, below @ref get_step_count.
	 * @return span<const memory_range> The stretches of that step.
	 */
	span<const memory_range> get_step_ranges(std::size_t step) const noexcept;

	/**
	 * @brief Get the first region one step covers.
	 *
	 * @param step Index of the step, below @ref get_step_count.
	 * @return std::size_t Its index among the regions, in the order the file
	 * offsets were given in.
	 */
	std::size_t get_step_first_region(std::size_t step) const noexcept;

	/**
	 * @brief Get how many regions one step covers.
	 *
	 * @param step Index of the step, below @ref get_step_count.
	 * @return std::size_t The number of regions, never zero.
	 */
	std::size_t get_step_region_count(std::size_t step) const noexcept;

private:
	std::vector<memory_range> m_ranges;
	std::vector<std::size_t> m_step_first_range;
	std::vector<std::size_t> m_step_first_region;
};

} // namespace mrc
} // namespace em
} // namespace rexlib
