// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/binary/bit.hpp>
#include <rexlib/core/binary/flagset.hpp>

#include <ostream>

namespace rexlib
{
namespace em
{

/**
 * @brief Properties an image reader advertises about how it serves reads.
 *
 * Every bit describes what a read costs, never what a read is allowed to do.
 * A reader always produces correct data for any in-bounds region regardless
 * of which of these are set; a consumer reads them to decide how to schedule
 * its work.
 */
enum class image_access_flag_bits {
	/**
	 * @brief Regions in increasing index order are in increasing file order.
	 *
	 * When set, sorting a batch of regions by index sorts it by position on
	 * the storage, so merging neighbouring regions into one larger read is
	 * worthwhile. A format that scatters its elements leaves it clear.
	 */
	ordered_offsets = bit(0),

	/**
	 * @brief Concurrent reads on one reader proceed in parallel.
	 *
	 * Reading concurrently is always safe. When this is clear, the reader
	 * still serves such calls correctly but serialises them internally, so
	 * spreading a batch over several threads buys nothing and a consumer is
	 * better off opening a reader per thread.
	 */
	concurrent_read = bit(1),

	/**
	 * @brief The file is already resident in the address space.
	 *
	 * Set by a reader backed by a mapping rather than by positioned reads,
	 * for which a read is a copy out of memory and no system call is paid.
	 */
	memory_resident = bit(2),
};

using image_access_flags = flagset<image_access_flag_bits>;

REXLIB_CONSTEXPR
const char* to_string(image_access_flag_bits v) noexcept;

template<typename T>
std::basic_ostream<T>&
operator<<(std::basic_ostream<T>& os, image_access_flag_bits v);

} // namespace em
} // namespace rexlib

#include "image_access_flags.inl"
