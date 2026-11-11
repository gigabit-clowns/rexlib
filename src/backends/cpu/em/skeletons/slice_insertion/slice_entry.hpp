// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/em/skeletons/slice_insertion/slice_frame.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief What a tile needs of one slice, settled once per call.
 */
class slice_entry
{
public:
	slice_entry() noexcept;

	/**
	 * @brief Construct the entry of a slice.
	 *
	 * @param frame Where the frequencies of the slice land.
	 * @param last_shell The last frequency shell with a nonzero radial
	 * weight, or -1 when there is none.
	 * @param slice_offset The offset of the slice in its operand, in
	 * elements.
	 * @param radial_offset The offset of the radial weights of the slice in
	 * their operand, in elements.
	 * @param phase_offset The offset of the phase tables of the slice, in
	 * elements.
	 */
	slice_entry(
		const slice_frame &frame,
		std::ptrdiff_t last_shell,
		std::ptrdiff_t slice_offset,
		std::ptrdiff_t radial_offset,
		std::size_t phase_offset
	) noexcept;

	const slice_frame& get_frame() const noexcept;
	std::ptrdiff_t get_last_shell() const noexcept;
	std::ptrdiff_t get_slice_offset() const noexcept;
	std::ptrdiff_t get_radial_offset() const noexcept;
	std::size_t get_phase_offset() const noexcept;

private:
	slice_frame m_frame;
	std::ptrdiff_t m_last_shell;
	std::ptrdiff_t m_slice_offset;
	std::ptrdiff_t m_radial_offset;
	std::size_t m_phase_offset;
};

} // namespace cpu
} // namespace rexlib
