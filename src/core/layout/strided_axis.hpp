// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/slice.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/attributes.hpp>

#include <cstddef>

namespace rexlib 
{

/**
 * @brief Class describing element layout on a tensor
 * axis
 * 
 */
class strided_axis 
{
public:
	/**
	 * @brief Constructor
	 * 
	 * @param extent Number of elements in the axis.
	 * @param stride Step between consecutive elements. In items.
	 */
	REXLIB_CONSTEXPR strided_axis(
		std::size_t extent, 
		std::ptrdiff_t stride
	) noexcept;

	strided_axis() = default;
	strided_axis(const strided_axis& other) = default;
	strided_axis(strided_axis&& other) = default;
	~strided_axis() = default;

	strided_axis& operator=(const strided_axis& other) = default;
	strided_axis& operator=(strided_axis&& other) = default;
	
	friend REXLIB_CONSTEXPR
	bool operator==(const strided_axis& lhs, const strided_axis& rhs) noexcept
	{
		return lhs.m_extent == rhs.m_extent && lhs.m_stride == rhs.m_stride;
	}

	friend REXLIB_CONSTEXPR
	bool operator!=(const strided_axis& lhs, const strided_axis& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	REXLIB_CONSTEXPR_CPP20 void swap(strided_axis &other) noexcept;

	/**
	 * @brief Get the hash value of the axis.
	 * 
	 * @return std::size_t The hash.
	 */
	std::size_t hash() const noexcept;

	/**
	 * @brief Set the element count
	 * 
	 * @param extent The element count
	 */
	REXLIB_CONSTEXPR void set_extent(std::size_t extent) noexcept;

	/**
	 * @brief Return the element count
	 * 
	 * @return std::size_t Element count
	 */
	REXLIB_CONSTEXPR std::size_t get_extent() const noexcept;

	/**
	 * @brief Set the stride between consecutive elements
	 * 
	 * @param stride Step between consecutive elements. In items
	 */
	REXLIB_CONSTEXPR void set_stride(std::ptrdiff_t stride) noexcept;

	/**
	 * @brief Get the stride between consecutive elements
	 * 
	 * @return std::ptrdiff_t Step between consecutive elements. In items
	 */
	REXLIB_CONSTEXPR std::ptrdiff_t get_stride() const noexcept;

	/**
	 * @brief Get the stride magnitude.
	 * 
	 * @return std::size_t Step between consecutive elements. In items
	 */
	REXLIB_CONSTEXPR std::size_t get_stride_magnitude() const noexcept;

private:
	std::size_t m_extent; ///< Number of elements
	std::ptrdiff_t m_stride; ///< Step between adjacent elements. In items
};

REXLIB_CONSTEXPR_CPP20 void swap(strided_axis &x, strided_axis &y) noexcept;

/**
 * @brief Create a contiguous strided_axis.
 * 
 * A contiguous axis descriptor has a unitary stride.
 * 
 * @param extent Number of elements on the axis.
 * @return strided_axis. The resulting axis
 */
REXLIB_CONSTEXPR
strided_axis make_contiguous_axis(std::size_t extent=1) noexcept;

/**
 * @brief Create a phantom strided_axis.
 * 
 * A phantom axis descriptor has null stride. 
 * This means that it does not contribute to the
 * underlying array's storage size and all its 
 * elements are repeated. 
 * 
 * @param extent Number of elements on the axis.
 * @return strided_axis. The resulting axis
 */
REXLIB_CONSTEXPR
strided_axis make_phantom_axis(std::size_t extent=1) noexcept;

/**
 * @brief Compare the absolute strides of a given pair of axes.
 * 
 * @param lhs Left hand side operand.
 * @param rhs Right hand side operand.
 * @return bool True if left hand side's absolute stride is equal to 
 * right hand side's stride.
 */
REXLIB_CONSTEXPR 
bool compare_strides_equal(
	const strided_axis &lhs, 
	const strided_axis &rhs
) noexcept;

/**
 * @brief Compare the absolute strides of a given pair of axes.
 * 
 * @param lhs Left hand side operand.
 * @param rhs Right hand side operand.
 * @return bool True if left hand side's absolute stride is less than 
 * right hand side's stride.
 */
REXLIB_CONSTEXPR 
bool compare_strides_less(
	const strided_axis &lhs, 
	const strided_axis &rhs
) noexcept;

/**
 * @brief Compare the absolute strides of a given pair of axes.
 * 
 * @param lhs Left hand side operand.
 * @param rhs Right hand side operand.
 * @return bool True if left hand side's absolute stride is greater than 
 * right hand side's stride.
 */
REXLIB_CONSTEXPR 
bool compare_strides_greater(
	const strided_axis &lhs, 
	const strided_axis &rhs
) noexcept;

/**
 * @brief Check if an axis is contiguous.
 * 
 * An axis is contiguous if its stride is one.
 * 
 * @param axis Axis to be checked.
 * @return true if the axis is contiguous.
 * @return false if the axis is not contiguous.
 */
REXLIB_CONSTEXPR
bool is_contiguous(const strided_axis &axis) noexcept;

/**
 * @brief Check if an axis pair is contiguous.
 * 
 * A pair of axes is contiguous if stride*extent of the
 * major axis is equal to the stride of the minor axis.
 * 
 * @param major Fast axis (smallest stride).
 * @param minor Slow axis (largest stride).
 * @return true if the axes are contiguous.
 * @return false if the axes are not contiguous.
 */
REXLIB_CONSTEXPR
bool is_contiguous(
	const strided_axis &major,
	const strided_axis &minor
) noexcept;

/**
 * @brief Check if an axis is reversed.
 * 
 * An axis is reversed if has negative stride.
 * 
 * @param axis Axis to be checked.
 * @return bool True if the axis is reversed.
 */
REXLIB_CONSTEXPR
bool is_reversed(const strided_axis &axis) noexcept;

/**
 * @brief Check if an axis is repeating.
 * 
 * An axis is repeating if it has an extent greater than one
 * and zero stride.
 * 
 * @param axis Axis to be checked.
 * @return bool True if the axis is repeating.
 */
REXLIB_CONSTEXPR
bool is_repeating(const strided_axis &axis) noexcept;

/**
 * @brief Check if an axis is empty
 * 
 * An axis is empty if it has an extent of zero.
 * 
 * @param axis The axis to be checked
 * @return true if the axis is empty
 * @return false if the axis is not empty.
 */
REXLIB_CONSTEXPR
bool is_empty(const strided_axis &axis) noexcept;

/**
 * @brief Check if an axis is significant.
 * 
 * An axis is significant if it has an extent not equal to 1.
 * 
 * @param axis The axis to be checked.
 * @return true if the axis is significant.
 * @return false if the axis is not significant.
 */
REXLIB_CONSTEXPR
bool is_significant(const strided_axis &axis) noexcept;

/**
 * @brief Get the axis last position referenced by ab axis.
 * 
 * @param axis The last position referenced by the axis.
 * @param result Output variable with the result. Only written when the axis
 * has non-zero extent.
 * @return true if the axis has non-zero extent and output was written.
 * @return false if the axis has an extent of zero.
 */
REXLIB_CONSTEXPR 
bool get_axis_last_offset(
	const strided_axis &axis, 
	std::ptrdiff_t &result
) noexcept;

/**
 * @brief Obtain the offset applied when reversing an axis.
 * 
 * If the axis is empty, this function returns 0 regardless of the stride.
 * If the axis is not reversed, this function returns 0.
 * If the axis is reversed, this function returns (extent-1)*unsigned_stride
 * of the axis.
 * 
 * @param axis Axis to be analyzed. 
 * @return std::size_t The offset.
*/
REXLIB_CONSTEXPR
std::size_t get_axis_pivot_offset(const strided_axis &axis) noexcept;

/**
 * @brief Perform a broadcast between two axes.
 * 
 * If both axes have the same extent, nothing is modified and true is returned.
 * If the first axis has an extent of 1, it is replaced with a phantom axis
 * with the extent of the second axis and true is returned. 
 * If the second axis has an extent of 1, it is replaced with a phantom axis
 * with the extent of the first axis and true is returned.
 * Otherwise, nothing is modified and false is returned.
 * 
 * @param axis1 An axis to be broadcasted.
 * @param axis2 An axis to be broadcasted.
 * @return true When successful.
 * @return false when unable to broadcast.
 */
REXLIB_CONSTEXPR
bool broadcast(strided_axis &axis1, strided_axis &axis2) noexcept;

/**
 * @brief Perform a broadcast of an axis to a specific extent.
 * 
 * If the axis' extent matches the provided extent, nothing is modified
 * and true is returned.
 * If the axis' extent does not match the provided extent but the former 
 * one is 1, the axis is replaced with a phantom axis with the provided 
 * extent, and true is returned.
 * If the axis' extent does not match the provided extent and is not 1, 
 * nothing is modified and false is returned.
 * 
 * @param axis The axis to be broadcasted.
 * @param extent The target extent for broadcasting.
 * @return true When successful.
 * @return false When unable to broadcast.
 */
REXLIB_CONSTEXPR
bool broadcast_to(strided_axis &axis, std::size_t extent) noexcept;

/**
 * @brief Apply an index to an axis to update an offset while checking bounds.
 * 
 * This is equivalent to calling sanitize_index and apply_index
 * 
 * @see apply_index
 * @see sanitize_index
 * @param axis Axis where the index is applied.
 * @param offset Offset to be updated.
 * @param index The index to be applied.
 * @throws std::out_ofr_range
 */
void apply_index(
	const strided_axis &axis, 
	std::ptrdiff_t &offset,
	std::ptrdiff_t index
);

/**
 * @brief Apply a slice to an axis to update an offset while checking for 
 * bounds.
 * 
 * This is equivalent to calling sanitize_slice and apply_slice
 * 
 * @see apply_slice
 * @see sanitize_slice
 * @param axis Axis where the index is applied. It is modified accordingly.
 * @param offset Offset to be updated.
 * @param slice The slice to be applied to the axis.
 * @throws std::out_ofr_range
 */
void apply_slice(
	strided_axis &axis,
	std::ptrdiff_t &offset,
	const slice &slice
);

} // namespace rexlib

#include "strided_axis.inl"
