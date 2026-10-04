// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief Size of the main header, in bytes.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t header_size = 1024;

/**
 * @brief Byte offset of each field of the main header.
 *
 * The offsets of the MRC2014 layout, which a header is read from and written
 * to. Their names are the ones the specification gives the fields.
 */
namespace offset
{

REXLIB_INLINE_CONST_CONSTEXPR std::size_t nx = 0;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t ny = 4;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nz = 8;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t mode = 12;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nxstart = 16;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nystart = 20;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nzstart = 24;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t mx = 28;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t my = 32;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t mz = 36;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t cella = 40;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t cellb = 52;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t mapc = 64;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t mapr = 68;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t maps = 72;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t dmin = 76;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t dmax = 80;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t dmean = 84;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t ispg = 88;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nsymbt = 92;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t extra1 = 96;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t exttyp = 104;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nversion = 108;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t extra2 = 112;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t imod_stamp = 152;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t imod_flags = 156;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t origin = 196;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t map = 208;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t machst = 212;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t rms = 216;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t nlabl = 220;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t label = 224;

} // namespace offset

/**
 * @brief Size of each fixed width byte array of the main header.
 */
namespace size
{

REXLIB_INLINE_CONST_CONSTEXPR std::size_t extra1 = 8;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t exttyp = 4;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t extra2 = 84;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t map = 4;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t machst = 4;
REXLIB_INLINE_CONST_CONSTEXPR std::size_t label = 80;

} // namespace size

/**
 * @brief Number of labels the main header carries.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t label_count = 10;

/**
 * @brief The identifier a file carries at @ref offset::map.
 *
 * Only its first three bytes are matched when a file is recognized: that is
 * the form the MRC2014 paper states, and some writers emit nothing more.
 */
REXLIB_INLINE_CONST_CONSTEXPR char map_id[] = "MAP ";

/**
 * @brief Bytes of @ref offset::map that identify a file.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::size_t map_id_match_size = 3;

/**
 * @brief First two bytes of the machine stamp of a little-endian file.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::array<std::uint8_t, 2>
little_endian_machine_stamp = {{0x44, 0x44}};

/**
 * @brief First two bytes of the machine stamp some little-endian files carry
 * in place of @ref little_endian_machine_stamp.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::array<std::uint8_t, 2>
legacy_little_endian_machine_stamp = {{0x44, 0x41}};

/**
 * @brief First two bytes of the machine stamp of a big-endian file.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::array<std::uint8_t, 2>
big_endian_machine_stamp = {{0x11, 0x11}};

/**
 * @brief Number of axes of space the columns, the rows and the sections of a
 * file run along.
 *
 * @ref offset::mapc, @ref offset::mapr and @ref offset::maps name one of them
 * each, counting from one.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t space_axis_count = 3;

/**
 * @brief Space group of a file holding one image or a stack of them.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t image_stack_space_group = 0;

/**
 * @brief Space group of a file holding one volume.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t volume_space_group = 1;

/**
 * @brief First space group of the range that denotes a stack of volumes.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t first_volume_stack_space_group =
	401;

/**
 * @brief Last space group of the range that denotes a stack of volumes.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t last_volume_stack_space_group =
	630;

/**
 * @brief Value of @ref offset::imod_stamp that makes
 * @ref offset::imod_flags meaningful.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t imod_stamp_value = 1146047817;

/**
 * @brief Bit of @ref offset::imod_flags stating that mode 0 holds signed
 * bytes.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t imod_signed_bytes_flag = 1;

/**
 * @brief Version this library writes, the year followed by the version
 * within it.
 */
REXLIB_INLINE_CONST_CONSTEXPR std::int32_t written_version = 20141;

/**
 * @brief Check whether a space group denotes a stack of volumes.
 *
 * @param space_group The space group to check.
 * @return bool true if it denotes a stack of volumes.
 */
bool is_volume_stack_space_group(std::int32_t space_group) noexcept;

} // namespace mrc
} // namespace em
} // namespace rexlib

#include "mrc_constants.inl"
