// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_scratch_runs.hpp"
#include "image_scratch_slots.hpp"

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>

#include <cstddef>
#include <mutex>

namespace rexlib
{
namespace em
{

/**
 * @brief A scratch entry that stores its copy in an array.
 *
 * The entry holds some indices of the first axis of a file. The array
 * stores them in slot order, in the data type of the file.
 *
 * Slots are loaded from the file one run at a time. Storing a region loads
 * every run that contains a held index of the region. Loads are serialised:
 * two threads never load the same run twice.
 *
 * A region is read from the entry only if all its indices are held and
 * loaded.
 */
class buffer_image_scratch_entry final
	: public image_scratch_entry
{
public:
	/**
	 * @brief Construct an entry with nothing loaded.
	 *
	 * @param slots The indices of the file to hold.
	 * @param values Storage for the held indices. Its first extent must be
	 * the number of slots. Its other extents and its data type must be
	 * those of the file.
	 * @param run_length Number of slots per run.
	 * @throws std::invalid_argument If @p values is not initialized, if its
	 * first extent is not the number of slots, or if @p run_length is zero.
	 * @throws unsupported_capability_error If @p values is not host
	 * accessible.
	 */
	buffer_image_scratch_entry(
		image_scratch_slots slots,
		array values,
		std::size_t run_length
	);

	~buffer_image_scratch_entry() override;

	image_transfer_plan read(
		array_ref destination,
		const image_transfer_plan &regions
	) const override;

	void store(
		const image_reader &file,
		const image_transfer_plan &regions
	) override;

private:
	/**
	 * @brief Check whether a range of consecutive indices is held and
	 * loaded.
	 *
	 * @param first_index First index of the range.
	 * @param index_count Number of indices in the range.
	 * @return true Every index of the range is held and loaded.
	 * @return false The range is empty, or an index of it is not held or
	 * not loaded.
	 */
	bool is_present(
		std::size_t first_index,
		std::size_t index_count
	) const noexcept;

	/**
	 * @brief Load a run from the file, unless it is already present.
	 *
	 * @param file A reader over the file.
	 * @param run Index of the run.
	 */
	void load(const image_reader &file, std::size_t run);

	/**
	 * @brief Make the plan that reads a run from the file into its slots.
	 *
	 * @param run Index of the run.
	 * @return image_transfer_plan One region per slot of the run.
	 */
	image_transfer_plan make_run_plan(std::size_t run) const;

	image_scratch_slots m_slots;
	image_scratch_runs m_runs;
	array m_values;
	std::mutex m_mutex;
};

} // namespace em
} // namespace rexlib
