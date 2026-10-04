// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>

#include <cstddef>
#include <cstdint>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief How a file states the type of the values it holds.
 *
 * The values the MRC2014 specification defines. Not all of them can be
 * transferred: @ref complex_int16 has no @ref numerical_type counterpart and
 * @ref packed_4bit is not addressable one element at a time, so
 * @ref is_supported rejects both.
 */
enum class mrc_mode : std::int32_t
{
	int8 = 0,
	int16 = 1,
	float32 = 2,
	complex_int16 = 3,
	complex_float32 = 4,
	uint16 = 6,
	float16 = 12,
	packed_4bit = 101,
};

/**
 * @brief Check whether a mode can be transferred.
 *
 * @param mode The mode to check.
 * @return bool true if the mode has a @ref numerical_type counterpart this
 * format can read and write.
 */
bool is_supported(mrc_mode mode) noexcept;

/**
 * @brief Get the data type a mode holds.
 *
 * @param mode The mode to resolve.
 * @param signed_bytes Whether @ref mrc_mode::int8 holds signed values. The
 * specification says it does, but a file carrying the IMOD stamp without its
 * signed byte flag holds unsigned ones. Ignored by every other mode.
 * @return numerical_type The data type, or @ref numerical_type::unknown for a
 * mode @ref is_supported rejects.
 */
numerical_type get_data_type(mrc_mode mode, bool signed_bytes) noexcept;

/**
 * @brief Get the size of one element of a mode.
 *
 * @param mode The mode to measure.
 * @return std::size_t The size in bytes, or zero for a mode
 * @ref is_supported rejects.
 */
std::size_t get_element_size(mrc_mode mode) noexcept;

/**
 * @brief Get the mode that holds a data type.
 *
 * @param type The data type to encode.
 * @return mrc_mode The mode holding it. Both @ref numerical_type::int8 and
 * @ref numerical_type::uint8 map onto @ref mrc_mode::int8, which is what
 * @ref needs_imod_unsigned_flag tells apart.
 * @throws unsupported_operation_error If no mode holds @p type.
 */
mrc_mode get_mode(numerical_type type);

/**
 * @brief Check whether writing a data type needs the IMOD unsigned byte flag.
 *
 * True only for @ref numerical_type::uint8, whose mode is the one the
 * specification declares signed. A file holding it says so through the IMOD
 * stamp with its signed byte flag clear, which is what @ref get_data_type
 * reads back.
 *
 * @param type The data type being written.
 * @return bool true if the flag has to be written.
 */
bool needs_imod_unsigned_flag(numerical_type type) noexcept;

} // namespace mrc
} // namespace em
} // namespace rexlib
