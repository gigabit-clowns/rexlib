// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>

#include <cstdint>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief Get the data type a sample of a given width and format holds.
 *
 * A TIFF file states the type of its samples as two tags, how many bits one
 * takes and how they are to be read: as an unsigned integer, a signed one or
 * a floating point number.
 *
 * @param bits_per_sample Width of one sample, in bits.
 * @param sample_format How the bits are read, as the SampleFormat tag
 * states it.
 * @return numerical_type The data type, or @ref numerical_type::unknown for
 * a pair that has no counterpart this format can transfer, which is every
 * sample narrower than a byte or wider than four, and every format beside
 * the three above.
 */
numerical_type get_data_type(
	std::uint16_t bits_per_sample,
	std::uint16_t sample_format
) noexcept;

/**
 * @brief Check whether a data type can be held by the samples of a file.
 *
 * @param type The data type to check.
 * @return bool true if @ref get_bits_per_sample and @ref get_sample_format
 * resolve it.
 */
bool is_supported(numerical_type type) noexcept;

/**
 * @brief Get the width of the samples that hold a data type.
 *
 * @param type The data type to encode.
 * @return std::uint16_t The width of one sample, in bits.
 * @throws unsupported_operation_error If no sample holds @p type.
 */
std::uint16_t get_bits_per_sample(numerical_type type);

/**
 * @brief Get the format of the samples that hold a data type.
 *
 * @param type The data type to encode.
 * @return std::uint16_t The value of the SampleFormat tag.
 * @throws unsupported_operation_error If no sample holds @p type.
 */
std::uint16_t get_sample_format(numerical_type type);

} // namespace tiff
} // namespace em
} // namespace rexlib
