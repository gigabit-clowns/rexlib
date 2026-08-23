// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "joint_layout_build_flags.hpp"
#include "joint_layout.hpp"
#include "../span.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>

namespace rex 
{

class strided_layout;
class joint_layout_implementation;

/**
 * @brief Factory class to obtain instances of array_access_layout.
 * 
 * @see array_access_layout
 * 
 */
class joint_layout_builder
{
public:
	static REX_CONST_CONSTEXPR 
	joint_layout_build_flags default_flags = {
		joint_layout_build_flag_bits::enable_reordering,
		joint_layout_build_flag_bits::enable_coalescing
	};

	REXLIB_API joint_layout_builder() noexcept;
	joint_layout_builder(
		const joint_layout_builder&
	) = delete;
	REXLIB_API
	joint_layout_builder(
		joint_layout_builder&& other
	) noexcept;
	REXLIB_API ~joint_layout_builder();

	joint_layout_builder& 
	operator=(const joint_layout_builder&) = delete;
	REXLIB_API joint_layout_builder& 
	operator=(joint_layout_builder&& other) noexcept;

	/**
	 * @brief Set the extents of the iteration space.
	 * 
	 * @param extents Extents of the iteration space.
	 * @return joint_layout_builder& A reference to *this.
	 * 
	 * @note This method can only be called once at most for each layout 
	 * creation and it must be called before any call to add_operand.
	 */
	REXLIB_API
	joint_layout_builder& 
	set_extents(span<const std::size_t> extents);

	/**
	 * @brief Add an operand to the layout.
	 *
	 * If set_extents has not been called, the first call to this method will
	 * set the extents of the operand as the extents of the iteration space.
	 * 
	 * When the iteration space extents are defined, the provided layout must
	 * be broadcastable to these extents.
	 *  
	 * @param layout Layout of the operand.
	 * @return joint_layout_builder& A reference to *this.
	 */
	REXLIB_API
	joint_layout_builder& 
	add_operand(const strided_layout &layout);

	/**
	 * @brief Add an operand to the layout.
	 *
	 * If set_extents has not been called, the first call to this method will
	 * set the extents of the operand as the extents of the iteration space.
	 * 
	 * When the iteration space extents are defined, the provided layout must
	 * be broadcastable to these extents.
	 * 
	 * @param extents The extents of the operand.
	 * @param strides The strides of the operand
	 * @param offset The offset of the operand.
	 * @return joint_layout_builder& A reference to *this.
	 */
	REXLIB_API
	joint_layout_builder& 
	add_operand(
		span<const std::size_t> extents,
		span<const std::ptrdiff_t> strides,
		std::ptrdiff_t offset
	);

	/**
	 * @brief Build the joint_layout from the operands contained in 
	 * this builder.
	 * 
	 * The building process can apply certain optimizations to the joint
	 * layout which can be controlled though the flags parameter.
	 * 
	 * @param flags Flags controlling the optimizations applied to the joint
	 * layout. By default, all optimizations are enabled.
	 * @return joint_layout The newly built joint_layout.
	 */
	REXLIB_API
	joint_layout build(
		joint_layout_build_flags flags = default_flags
	);

	/**
	 * @brief Get a pointer to the implementation.
	 * 
	 * @return const joint_layout_implementation* Pointer to the
	 * implementation.
	 * 
	 * @note This method is meant to be used for testing purposes, as the 
	 * joint_layout_implementation is not publicly available.
	 */
	const joint_layout_implementation* 
	get_implementation() const noexcept;

private:
	std::unique_ptr<joint_layout_implementation> m_implementation;
};

} // namespace rex
