// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_prefetch_policy.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/core/system/page_prefetch.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class image_transfer_plan;

/**
 * @brief Work out what one region of a batch spans, in bytes.
 *
 * The stretch from the first byte of a region to its last, gaps included,
 * which is what a region occupies when it is not contiguous. Every region of
 * a batch shares the extents, so all of them span the same.
 *
 * @param regions The regions to be moved.
 * @param file_strides Distance between consecutive elements of the file along
 * each axis, in elements.
 * @param data_type Data type of the elements of the file.
 * @return std::size_t The span in bytes, zero for a region of no elements.
 */
std::size_t compute_region_span(
	const image_transfer_plan &regions,
	span<const std::ptrdiff_t> file_strides,
	numerical_type data_type
) noexcept;

/**
 * @brief The stretches of a file a batch reaches, grouped into steps.
 *
 * The regions of a batch are advised as the stretches they actually touch
 * rather than as the one span that covers them, so that a batch drawn at
 * random out of a large file asks for what it reads and not for the file.
 * Stretches that overlap or that lie within the tolerance of each other are
 * merged, which collapses a batch reading consecutive regions back into the
 * few stretches it is. Merging stops where a stretch would grow past the
 * budget of a step, so that a long run of consecutive regions is advised a
 * step at a time like any other batch rather than all at once.
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
 * The steps tile the batch: every region belongs to exactly one of them, so
 * walking the steps walks the batch. A region there is nothing to ask for,
 * one that starts past what is mapped, still belongs to a step, since advice
 * is what this decides and moving the values is not.
 *
 * The file offsets this is built from must be ascending, which is how
 * @ref image_region_offsets holds them.
 */
class image_region_prefetch_plan
{
public:
	/**
	 * @brief Work out the stretches of a batch and the steps they fall in.
	 *
	 * @param regions The regions to be moved.
	 * @param file_strides Distance between consecutive elements of the file
	 * along each axis, in elements.
	 * @param data_type Data type of the elements of the file.
	 * @param file_offsets Where each region starts in the file, in elements,
	 * ascending.
	 * @param mapping The bytes the file is mapped at, from its first one,
	 * which no stretch reaches past.
	 * @param data_offset Where the values of the file begin in @p mapping,
	 * in bytes.
	 * @param policy What the batch is advised with.
	 */
	image_region_prefetch_plan(
		const image_transfer_plan &regions,
		span<const std::ptrdiff_t> file_strides,
		numerical_type data_type,
		span<const std::ptrdiff_t> file_offsets,
		span<byte> mapping,
		std::size_t data_offset,
		const image_prefetch_policy &policy
	);

	image_region_prefetch_plan(
		const image_region_prefetch_plan &other
	) = default;
	image_region_prefetch_plan(
		image_region_prefetch_plan &&other
	) noexcept = default;
	~image_region_prefetch_plan() = default;

	image_region_prefetch_plan&
	operator=(const image_region_prefetch_plan &other) = default;
	image_region_prefetch_plan&
	operator=(image_region_prefetch_plan &&other) noexcept = default;

	/**
	 * @brief Get how many steps the batch is walked in.
	 *
	 * @return std::size_t The number of steps, zero for a batch of no region.
	 */
	std::size_t get_step_count() const noexcept;

	/**
	 * @brief Get every stretch the batch reaches.
	 *
	 * @return span<const memory_range> The stretches, ascending and disjoint.
	 */
	span<const memory_range> get_ranges() const noexcept;

	/**
	 * @brief Get the stretches one step advises.
	 *
	 * @param step Index of the step, below @ref get_step_count.
	 * @return span<const memory_range> The stretches of that step, empty
	 * where there is nothing to ask for.
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
	/**
	 * @brief Work out the stretches of the batch, merging as they are found.
	 *
	 * @return std::vector<std::size_t> How many regions each stretch covers.
	 */
	std::vector<std::size_t> gather_ranges(
		const image_transfer_plan &regions,
		span<const std::ptrdiff_t> file_strides,
		numerical_type data_type,
		span<const std::ptrdiff_t> file_offsets,
		span<byte> mapping,
		std::size_t data_offset,
		const image_prefetch_policy &policy
	);

	/**
	 * @brief Group the stretches into steps of a bounded number of bytes.
	 *
	 * @param regions_per_range How many regions each stretch covers.
	 * @param region_count How many regions the batch holds, which the last
	 * step reaches whether or not every one of them was asked for.
	 * @param byte_budget Most bytes one step advises.
	 */
	void gather_steps(
		span<const std::size_t> regions_per_range,
		std::size_t region_count,
		std::size_t byte_budget
	);

	std::vector<memory_range> m_ranges;
	std::vector<std::size_t> m_step_first_range;
	std::vector<std::size_t> m_step_first_region;
};

} // namespace em
} // namespace rexlib
