// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <tiffio.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace rexlib
{
namespace test
{

/**
 * @brief Make the values 0, 1, 2 and onwards from a first one.
 *
 * @tparam T Type of the values.
 * @param count How many values to make.
 * @param first The first value.
 * @return std::vector<T> The values.
 */
template <typename T>
std::vector<T> counting(std::size_t count, T first = T(0))
{
	std::vector<T> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<T>(first + static_cast<T>(i));
	}

	return values;
}

/**
 * @brief Set the tags every page of the files built here carries.
 *
 * @tparam T Type of one sample, which settles how wide it is.
 * @param file The file being written.
 * @param width Number of columns of the page.
 * @param height Number of rows of the page.
 * @param sample_format The SampleFormat tag.
 * @param compression The Compression tag.
 */
template <typename T>
void set_page_tags(
	TIFF *file,
	std::uint32_t width,
	std::uint32_t height,
	std::uint16_t sample_format,
	std::uint16_t compression
)
{
	TIFFSetField(file, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(file, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, static_cast<int>(8 * sizeof(T)));
	TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(file, TIFFTAG_SAMPLEFORMAT, sample_format);
	TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(file, TIFFTAG_COMPRESSION, compression);
}

/**
 * @brief Write a file of pages cut into strips, through libtiff itself.
 *
 * @tparam T Type of one sample.
 * @param path Path to the file.
 * @param mode The mode libtiff creates it with: "w" for a classic file in
 * the byte order of the host, "wl" and "wb" for one of a stated byte order,
 * and "w8" for a BigTIFF one.
 * @param width Number of columns of every page.
 * @param height Number of rows of every page.
 * @param sample_format The SampleFormat tag.
 * @param compression The Compression tag.
 * @param rows_per_strip Number of rows of every strip but the last.
 * @param pages The samples of each page, row after row.
 */
template <typename T>
void write_striped_file(
	const std::string &path,
	const char *mode,
	std::uint32_t width,
	std::uint32_t height,
	std::uint16_t sample_format,
	std::uint16_t compression,
	std::uint32_t rows_per_strip,
	const std::vector<std::vector<T>> &pages
)
{
	auto *file = TIFFOpen(path.c_str(), mode);
	if (file == nullptr)
	{
		throw std::runtime_error("The test file could not be created.");
	}

	for (const auto &page : pages)
	{
		set_page_tags<T>(file, width, height, sample_format, compression);
		TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, rows_per_strip);

		// libtiff swaps the samples it is given in place when the file is
		// in the other byte order, so it is handed a copy.
		auto samples = page;
		std::uint32_t strip = 0;
		for (std::uint32_t row = 0; row < height; row += rows_per_strip)
		{
			const auto rows = std::min(rows_per_strip, height - row);
			TIFFWriteEncodedStrip(
				file,
				strip,
				samples.data() + static_cast<std::size_t>(row) * width,
				static_cast<tmsize_t>(rows * width * sizeof(T))
			);
			++strip;
		}

		TIFFWriteDirectory(file);
	}

	TIFFClose(file);
}

/**
 * @brief Write a file of one page cut into tiles, through libtiff itself.
 *
 * @tparam T Type of one sample.
 * @param path Path to the file.
 * @param width Number of columns of the page.
 * @param height Number of rows of the page.
 * @param sample_format The SampleFormat tag.
 * @param compression The Compression tag.
 * @param tile_width Number of columns of a tile, a multiple of 16.
 * @param tile_height Number of rows of a tile, a multiple of 16.
 * @param page The samples of the page, row after row.
 */
template <typename T>
void write_tiled_file(
	const std::string &path,
	std::uint32_t width,
	std::uint32_t height,
	std::uint16_t sample_format,
	std::uint16_t compression,
	std::uint32_t tile_width,
	std::uint32_t tile_height,
	const std::vector<T> &page
)
{
	auto *file = TIFFOpen(path.c_str(), "w");
	if (file == nullptr)
	{
		throw std::runtime_error("The test file could not be created.");
	}

	set_page_tags<T>(file, width, height, sample_format, compression);
	TIFFSetField(file, TIFFTAG_TILEWIDTH, tile_width);
	TIFFSetField(file, TIFFTAG_TILELENGTH, tile_height);

	std::vector<T> tile(static_cast<std::size_t>(tile_width) * tile_height);
	std::uint32_t index = 0;
	for (std::uint32_t top = 0; top < height; top += tile_height)
	{
		for (std::uint32_t left = 0; left < width; left += tile_width)
		{
			std::fill(tile.begin(), tile.end(), T(0));
			const auto rows = std::min(tile_height, height - top);
			const auto columns = std::min(tile_width, width - left);
			for (std::uint32_t row = 0; row < rows; ++row)
			{
				const auto *source = page.data() +
					static_cast<std::size_t>(top + row) * width + left;
				std::copy(
					source,
					source + columns,
					tile.begin() + static_cast<std::size_t>(row) * tile_width
				);
			}

			TIFFWriteEncodedTile(
				file,
				index,
				tile.data(),
				static_cast<tmsize_t>(tile.size() * sizeof(T))
			);
			++index;
		}
	}

	TIFFWriteDirectory(file);
	TIFFClose(file);
}

/**
 * @brief Write a file of one page of three 8 bit samples per pixel.
 *
 * @param path Path to the file.
 * @param width Number of columns of the page.
 * @param height Number of rows of the page.
 */
inline void write_rgb_file(
	const std::string &path,
	std::uint32_t width,
	std::uint32_t height
)
{
	auto *file = TIFFOpen(path.c_str(), "w");
	if (file == nullptr)
	{
		throw std::runtime_error("The test file could not be created.");
	}

	TIFFSetField(file, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(file, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, height);

	std::vector<std::uint8_t> samples(
		static_cast<std::size_t>(width) * height * 3, 0);
	TIFFWriteEncodedStrip(
		file, 0, samples.data(), static_cast<tmsize_t>(samples.size()));
	TIFFWriteDirectory(file);
	TIFFClose(file);
}

/**
 * @brief Read every byte of a file.
 *
 * @param path Path to the file.
 * @return std::vector<char> The bytes.
 */
inline std::vector<char> read_file(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
	return std::vector<char>(
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>()
	);
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

} // namespace test
} // namespace rexlib
