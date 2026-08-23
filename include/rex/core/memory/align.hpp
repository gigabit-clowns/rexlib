// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/attributes.hpp"
#include "../platform/constexpr.hpp"

#include <cstdint>
#include <cstddef>

namespace rex
{

/**
 * @brief Get the alignment of a pointer
 * 
 * Finds the largest power of two that is a divisor of the address
 * 
 * @tparam T Pointed value type
 * @param address The address of which the alignment is determined
 * @return std::size_t The alignment (a power of two)
 */
template <typename T>
REX_NODISCARD 
std::size_t get_alignment(T* address) noexcept;

/**
 * @brief Get the alignment of an address
 * 
 * Finds the largest power of two that is a divisor of the address
 * 
 * @param address The address of which the alignment is determined
 * @return std::size_t The alignment (a power of two)
 */
REX_NODISCARD REX_CONSTEXPR 
std::size_t get_alignment(std::uintptr_t address) noexcept;

/**
 * @brief Check if a pointer is aligned to the provided boundary.
 * 
 * @tparam T Pointed value type
 * @param address The address to be checked.
 * @param alignment The alignment requirement to be checked.
 * @return true If the pointer is aligned.
 * @return false If the pointer is not aligned.
 */
template <typename T>
REX_NODISCARD 
bool is_aligned(T* address, std::size_t alignment) noexcept;

/**
 * @brief Check if an address is aligned to the provided boundary.
 * 
 * @param address The address to be checked.
 * @param alignment The alignment requirement to be checked.
 * @return std::size_t The alignment (a power of two)
 */
REX_NODISCARD REX_CONSTEXPR 
bool is_aligned(std::uintptr_t address, std::size_t alignment) noexcept;

/**
 * @brief Align a pointer to a lower or equal address that satisfies the
 * provided alignment
 * 
 * @tparam T Pointed value type
 * @param address The address to be aligned
 * @param alignment The minimum alignment
 * @return T* Aligned pointer
 */
template <typename T>
REX_NODISCARD 
T* align_floor(T* address, std::size_t alignment) noexcept;

/**
 * @brief Align an address to a lower or equal address that satisfies the
 * provided alignment
 * 
 * @param address The address to be aligned
 * @param alignment The minimum alignment
 * @return std::uintptr_t Aligned address
 */
REX_NODISCARD REX_CONSTEXPR 
std::uintptr_t align_floor(std::uintptr_t address, std::size_t alignment) noexcept;

/**
 * @brief Align an address to a lower or equal address that satisfies the
 * provided alignment
 * 
 * @param address The address to be aligned
 * @param alignment The minimum alignment
 * @return std::uintptr_t Aligned address
 * @note The result is written in place and a reference is returned
 */
REX_CONSTEXPR 
std::uintptr_t& align_floor_inplace(std::uintptr_t& address, std::size_t alignment) noexcept;

/**
 * @brief Align a pointer to a higher or equal address that satisfies the
 * provided alignment
 * 
 * @tparam T Pointed value type
 * @param address The address to be aligned
 * @param alignment The minimum alignment
 * @return T* Aligned pointer
 */

template <typename T>
REX_NODISCARD 
T* align_ceil(T* address, std::size_t alignment) noexcept;

/**
 * @brief Align an address to a higher or equal address that satisfies the
 * provided alignment
 * 
 * @param address The address to be aligned
 * @param alignment The minimum alignment
 * @return std::uintptr_t Aligned address
 */
REX_NODISCARD REX_CONSTEXPR 
std::uintptr_t align_ceil(std::uintptr_t address, std::size_t alignment) noexcept;

/**
 * @brief Align an address to a higher or equal address that satisfies the
 * provided alignment
 * 
 * @param address The address to be aligned
 * @param alignment The minimum alignment
 * @return std::uintptr_t Aligned address
 * @note The result is written in place and a reference is returned
 */
REX_CONSTEXPR 
std::uintptr_t& align_ceil_inplace(std::uintptr_t& address, std::size_t alignment) noexcept;

/**
 * @brief Offset an address by a given amount of bytes
 * 
 * @tparam T Pointed type.
 * @param address Pointer to be modified.
 * @param count Number of bytes to add.
 * @return T* Pointer with offset.
 * 
 * @warning This may break alignment requirements. Of the underlying type.
 * Use it under your own risk 
 * 
 */
template <typename T>
REX_NODISCARD
T* offset_bytes(T* address, std::ptrdiff_t count);

} // namespace rex

#include "align.inl"
