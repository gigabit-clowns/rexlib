// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../binary/bit.hpp"
#include "../binary/flagset.hpp"

#include <ostream>

namespace xmipp4 
{

/**
 * @brief Flags to control the behavior of the joint_layout build 
 * behavior.
 * 
 */
enum class joint_layout_build_flag_bits
{
	enable_reordering = bit(0), ///< Reorder axes by memory locality.
	enable_coalescing = bit(1), ///< Coalesce contiguous axes.
};

/**
 * @brief Set of joint_layout_build_flag_bits used to control the 
 * joint_layout build behavior.
 * 
 */
using joint_layout_build_flags 
	= flagset<joint_layout_build_flag_bits>;

REX_CONSTEXPR 
const char* to_string(joint_layout_build_flag_bits x) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(
	std::basic_ostream<T>& os, 
	joint_layout_build_flag_bits x
);

} // namespace xmipp4

#include "joint_layout_build_flags.inl"
