// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/layout/joint_layout.hpp>
#include <xmipp4/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rex
{

class operand_signature;

namespace cpu
{

/**
 * @brief Which of the two domains each operand of a transform lives in.
 *
 * The three shapes a discrete Fourier transform comes in. They differ in
 * what relates the extents of the two operands, which is why the plan needs
 * to be told: a spectrum stored in half is the same array of numbers as one
 * stored whole, and only the operation knows which it is holding.
 */
enum class fourier_transform_kind
{
	/// Both operands are complex and share their extents.
	complex_to_complex,

	/// A real operand becomes the half spectrum of a complex one.
	real_to_complex,

	/// A half spectrum becomes a real operand.
	complex_to_real
};

/**
 * @brief The transform a Fourier program performs, settled at build time.
 *
 * The transform itself is delegated to pocketfft, which asks for the shape of
 * the signal, the strides of both operands **in bytes** and the axes to
 * transform along. All three are read off the operand signatures once here,
 * rather than every time the program runs, and are held in the very types
 * pocketfft takes them in so that running the program only has to hand them
 * over.
 *
 * The offsets are kept apart from the strides because they belong to the
 * pointers rather than to the traversal: whoever runs the program displaces
 * the operand pointers by them and lets everything else describe an array
 * starting where it points.
 *
 * @see fourier_program_builder
 */
class fourier_layout_plan
{
public:
	/**
	 * @brief Plan the transform of one operand into another.
	 *
	 * @param output_signature Signature of the transformed operand.
	 * @param input_signature Signature of the operand being transformed.
	 * @param axes The transformed axes, ascending and without repetitions.
	 * The last of them is the one a half spectrum is stored along.
	 * @param kind Which domain each operand lives in.
	 *
	 * @throws std::invalid_argument When the two operands do not have the
	 * extents @p kind implies, when a transform needing an axis is given
	 * none, or when a transformed axis is empty.
	 * @throws std::out_of_range When an axis is not within the rank of the
	 * operands.
	 */
	fourier_layout_plan(
		const operand_signature &output_signature,
		const operand_signature &input_signature,
		span<const std::size_t> axes,
		fourier_transform_kind kind
	);

	fourier_layout_plan(const fourier_layout_plan &other) = delete;
	fourier_layout_plan(fourier_layout_plan &&other) noexcept = default;
	~fourier_layout_plan() = default;

	fourier_layout_plan&
	operator=(const fourier_layout_plan &other) = delete;
	fourier_layout_plan&
	operator=(fourier_layout_plan &&other) noexcept = default;

	/**
	 * @brief Get the extents of the signal.
	 *
	 * The shape of whichever operand is not a half spectrum, which is the
	 * one both a real transform and its inverse are described by: the halved
	 * extent follows from it, and it is the extent the transform is
	 * normalised by.
	 *
	 * @return const std::vector<std::size_t>& The extents.
	 */
	const std::vector<std::size_t>& get_shape() const noexcept;

	/**
	 * @brief Get the transformed axes, ascending.
	 *
	 * @return const std::vector<std::size_t>& The axes.
	 */
	const std::vector<std::size_t>& get_axes() const noexcept;

	/**
	 * @brief Get the strides of the operand being transformed, in bytes.
	 *
	 * @return const std::vector<std::ptrdiff_t>& The strides.
	 */
	const std::vector<std::ptrdiff_t>& get_input_strides() const noexcept;

	/**
	 * @brief Get the strides of the transformed operand, in bytes.
	 *
	 * @return const std::vector<std::ptrdiff_t>& The strides.
	 */
	const std::vector<std::ptrdiff_t>& get_output_strides() const noexcept;

	/**
	 * @brief Get the offset of the operand being transformed.
	 *
	 * @return std::ptrdiff_t The offset, in elements.
	 */
	std::ptrdiff_t get_input_offset() const noexcept;

	/**
	 * @brief Get the offset of the transformed operand.
	 *
	 * @return std::ptrdiff_t The offset, in elements.
	 */
	std::ptrdiff_t get_output_offset() const noexcept;

	/**
	 * @brief Get the number of samples reaching each transformed value.
	 *
	 * The product of the signal extents along the transformed axes, which is
	 * what an inverse transform divides by to undo its forward. It is one
	 * when no axis is transformed and never zero, an empty transformed axis
	 * being turned away at construction.
	 *
	 * @return std::size_t The number of samples.
	 */
	std::size_t get_sample_count() const noexcept;

	/**
	 * @brief Get the layout pairing the two operands element by element.
	 *
	 * Only built when the operands share their extents, which is to say for
	 * @ref fourier_transform_kind::complex_to_complex. It is what a real
	 * operand is widened into its complex output through, and what a
	 * transform along no axis at all degenerates to.
	 *
	 * Its operands are the output followed by the input, and both start at
	 * zero: the offsets belong to the pointers handed to the loop, which are
	 * the same ones the transform is given.
	 *
	 * @return const joint_layout& The layout.
	 */
	const joint_layout& get_conversion_layout() const noexcept;

private:
	std::vector<std::size_t> m_shape;
	std::vector<std::size_t> m_axes;
	std::vector<std::ptrdiff_t> m_input_strides;
	std::vector<std::ptrdiff_t> m_output_strides;
	std::ptrdiff_t m_input_offset;
	std::ptrdiff_t m_output_offset;
	std::size_t m_sample_count;
	joint_layout m_conversion_layout;
};

} // namespace cpu
} // namespace rex
