// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

namespace rexlib
{
namespace em
{

/**
 * @brief Tracks which runs of slots are present in a scratch entry.
 *
 * The slots are split into runs of consecutive slots. All runs have the
 * same length, except the last one, which may be shorter. Every run starts
 * absent. Once a run is marked present, it stays present.
 *
 * @par Thread safety
 * @ref is_present and @ref mark_present may be called concurrently. A
 * thread that sees a run present also sees the data written before the run
 * was marked.
 */
class image_scratch_runs
{
public:
	/**
	 * @brief Split a number of slots into runs. All runs start absent.
	 *
	 * @param slot_count Number of slots.
	 * @param run_length Number of slots per run. If it exceeds
	 * @p slot_count, there is a single run.
	 * @throws std::invalid_argument If @p run_length is zero.
	 */
	image_scratch_runs(std::size_t slot_count, std::size_t run_length);

	image_scratch_runs(const image_scratch_runs &other) = delete;
	image_scratch_runs(image_scratch_runs &&other) noexcept = default;
	~image_scratch_runs() = default;

	image_scratch_runs& operator=(const image_scratch_runs &other) = delete;
	image_scratch_runs&
	operator=(image_scratch_runs &&other) noexcept = default;

	/**
	 * @brief Get the run that contains a slot.
	 *
	 * @param slot The slot. Must be below the number of slots.
	 * @return std::size_t Index of the run.
	 */
	std::size_t get_run(std::size_t slot) const noexcept;

	/**
	 * @brief Get the first slot of a run.
	 *
	 * @param run Index of the run. Must be a value returned by
	 * @ref get_run.
	 * @return std::size_t The slot.
	 */
	std::size_t get_first_slot(std::size_t run) const noexcept;

	/**
	 * @brief Get the number of slots in a run.
	 *
	 * @param run Index of the run. Must be a value returned by
	 * @ref get_run.
	 * @return std::size_t The number of slots. Never zero.
	 */
	std::size_t get_slot_count(std::size_t run) const noexcept;

	/**
	 * @brief Check whether a run is present.
	 *
	 * @param run Index of the run. Must be a value returned by
	 * @ref get_run.
	 * @return true The run was marked present.
	 * @return false The run is absent.
	 */
	bool is_present(std::size_t run) const noexcept;

	/**
	 * @brief Mark a run as present.
	 *
	 * @param run Index of the run. Must be a value returned by
	 * @ref get_run.
	 */
	void mark_present(std::size_t run) noexcept;

private:
	std::size_t m_slot_count;
	std::size_t m_run_length;
	std::unique_ptr<std::atomic<bool>[]> m_present;
};

} // namespace em
} // namespace rexlib
