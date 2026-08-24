// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/constexpr.hpp"
#include "binary/bit.hpp"

#include <ostream>
#include <cstdint>

namespace rex 
{

class version
{
public:
	version() = default;
	REXLIB_CONSTEXPR version(
		std::uint32_t major, 
		std::uint32_t minor, 
		std::uint32_t patch
	) noexcept;
	version(const version& other) = default;
	~version() = default;

	version& operator=(const version& other) = default;

	REXLIB_CONSTEXPR void set_major(std::uint32_t major) noexcept;
	REXLIB_CONSTEXPR std::uint32_t get_major() const noexcept;

	REXLIB_CONSTEXPR void set_minor(std::uint32_t minor) noexcept;
	REXLIB_CONSTEXPR std::uint32_t get_minor() const noexcept;

	REXLIB_CONSTEXPR void set_patch(std::uint32_t patch) noexcept;
	REXLIB_CONSTEXPR std::uint32_t get_patch() const noexcept;

	REXLIB_CONSTEXPR std::uint32_t get_data() const noexcept;

	friend REXLIB_CONSTEXPR
	bool operator==(const version& lhs, const version& rhs) noexcept
	{
		return lhs.get_data() == rhs.get_data();
	}

	friend REXLIB_CONSTEXPR
	bool operator!=(const version& lhs, const version& rhs) noexcept
	{
		return lhs.get_data() != rhs.get_data();
	}

	friend REXLIB_CONSTEXPR
	bool operator<(const version& lhs, const version& rhs) noexcept
	{
		return lhs.get_data() < rhs.get_data();
	}

	friend REXLIB_CONSTEXPR
	bool operator<=(const version& lhs, const version& rhs) noexcept
	{
		return lhs.get_data() <= rhs.get_data();
	}

	friend REXLIB_CONSTEXPR
	bool operator>(const version& lhs, const version& rhs) noexcept
	{
		return lhs.get_data() > rhs.get_data();
	}

	friend REXLIB_CONSTEXPR
	bool operator>=(const version& lhs, const version& rhs) noexcept
	{
		return lhs.get_data() >= rhs.get_data();
	}

	template<typename T>
	friend std::basic_ostream<T>&
	operator<<(std::basic_ostream<T>& os, const version& ver)
	{
		REXLIB_CONST_CONSTEXPR T sep = '.';
		return os
			<< ver.get_major() << sep
			<< ver.get_minor() << sep
			<< ver.get_patch();
	}

private:
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t patch_bits = 10;
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t minor_bits = 10;
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t major_bits = 12;

	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t patch_offset = 0;
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t minor_offset = patch_offset + patch_bits;
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t major_offset = minor_offset + minor_bits;

	static REXLIB_INLINE_CONST_CONSTEXPR std::uint32_t patch_mask = 
		bit_range_mask<std::uint32_t>(patch_offset, patch_offset+patch_bits);
	static REXLIB_INLINE_CONST_CONSTEXPR std::uint32_t minor_mask = 
		bit_range_mask<std::uint32_t>(minor_offset, minor_offset+minor_bits);
	static REXLIB_INLINE_CONST_CONSTEXPR std::uint32_t major_mask = 
		bit_range_mask<std::uint32_t>(major_offset, major_offset+major_bits);

	std::uint32_t m_data;
};

} // namespace rex

#include "version.inl"
