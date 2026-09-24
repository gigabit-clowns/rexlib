// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <em/image/formats/mrc/mrc_constants.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace rexlib
{
namespace test
{

/**
 * @brief Write one 32-bit field of a header in the byte order of the host.
 *
 * @param raw The bytes of the file.
 * @param offset Where the field begins.
 * @param value The value of the field.
 */
inline void put_int32(
	std::vector<char> &raw,
	std::size_t offset,
	std::int32_t value
)
{
	std::memcpy(raw.data() + offset, &value, sizeof(value));
}

/**
 * @brief Lay out a little-endian MRC file of 32-bit float values.
 *
 * Laid out for a host of that byte order, which is what every machine the
 * suite runs on is.
 *
 * @param columns Number of columns.
 * @param rows Number of rows.
 * @param sections Number of sections.
 * @param space_group The space group of the file.
 * @param mode The mode of the file.
 * @param values The values the file holds.
 * @return std::vector<char> The bytes of the file.
 */
inline std::vector<char> make_file(
	std::int32_t columns,
	std::int32_t rows,
	std::int32_t sections,
	std::int32_t space_group,
	std::int32_t mode,
	const std::vector<float> &values
)
{
	const auto header_size = em::mrc::header_size;
	std::vector<char> raw(header_size + values.size() * sizeof(float), '\0');

	put_int32(raw, 0, columns);
	put_int32(raw, 4, rows);
	put_int32(raw, 8, sections);
	put_int32(raw, 12, mode);
	put_int32(raw, 36, space_group == 0 ? 1 : sections);
	put_int32(raw, 64, 1);
	put_int32(raw, 68, 2);
	put_int32(raw, 72, 3);
	put_int32(raw, 88, space_group);
	std::memcpy(raw.data() + 208, "MAP ", 4);
	raw[212] = static_cast<char>(0x44);
	raw[213] = static_cast<char>(0x44);

	std::memcpy(
		raw.data() + header_size,
		values.data(),
		values.size() * sizeof(float)
	);

	return raw;
}

/**
 * @brief Write bytes to a file, replacing whatever it held.
 *
 * @param path Path to the file.
 * @param raw The bytes to write.
 */
inline void write_file(const std::string &path, const std::vector<char> &raw)
{
	std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
	output.write(raw.data(), static_cast<std::streamsize>(raw.size()));
}

/**
 * @brief Make the values 0, 1, 2 and onwards.
 *
 * @param count How many values to make.
 * @return std::vector<float> The values.
 */
inline std::vector<float> counting(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(i);
	}

	return values;
}

} // namespace test
} // namespace rexlib
