// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_signature.hpp"

#include <cstddef>
#include <cstdint>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

const std::size_t signature_size = 4;
const std::uint8_t little_endian_mark = 0x49;
const std::uint8_t big_endian_mark = 0x4D;
const std::uint8_t classic_version = 42;
const std::uint8_t big_version = 43;

bool is_version(std::uint8_t low, std::uint8_t high) noexcept
{
	return high == 0 && (low == classic_version || low == big_version);
}

} // anonymous namespace

bool has_signature(span<const byte> leading_bytes) noexcept
{
	if (leading_bytes.size() < signature_size)
	{
		return false;
	}

	const auto mark = as_uint8(leading_bytes[0]);
	if (as_uint8(leading_bytes[1]) != mark)
	{
		return false;
	}

	const auto third = as_uint8(leading_bytes[2]);
	const auto fourth = as_uint8(leading_bytes[3]);

	if (mark == little_endian_mark)
	{
		return is_version(third, fourth);
	}

	if (mark == big_endian_mark)
	{
		return is_version(fourth, third);
	}

	return false;
}

} // namespace tiff
} // namespace em
} // namespace rexlib
