// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_file.hpp"

#include "tiff_sample_type.hpp"

#include <core/logger.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <new>
#include <stdexcept>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

std::string format_message(const char *format, va_list arguments)
{
	std::array<char, 512> text = {};
	std::vsnprintf(text.data(), text.size(), format, arguments);
	return text.data();
}

int keep_error(
	TIFF */*handle*/,
	void *user_data,
	const char */*module*/,
	const char *format,
	va_list arguments
)
{
	*static_cast<std::string*>(user_data) =
		format_message(format, arguments);
	return 1;
}

int log_warning(
	TIFF */*handle*/,
	void */*user_data*/,
	const char */*module*/,
	const char *format,
	va_list arguments
)
{
	const std::string message = format_message(format, arguments);
	REXLIB_LOG_DEBUG("libtiff: {}", message);
	return 1;
}

const char* to_libtiff_mode(tiff_file_mode mode) noexcept
{
	switch (mode)
	{
	case tiff_file_mode::create: return "w";
	case tiff_file_mode::create_big: return "w8";
	default: return "r";
	}
}

std::uint16_t to_libtiff_compression(tiff_compression compression) noexcept
{
	switch (compression)
	{
	case tiff_compression::lzw: return COMPRESSION_LZW;
	default: return COMPRESSION_NONE;
	}
}

bool is_readable(const std::string &path)
{
	return std::ifstream(path.c_str(), std::ios::in | std::ios::binary)
		.is_open();
}

TIFF* open_handle(
	const std::string &path,
	tiff_file_mode mode,
	std::string &last_error
)
{
	auto *options = TIFFOpenOptionsAlloc();
	if (options == nullptr)
	{
		throw std::bad_alloc();
	}

	TIFFOpenOptionsSetErrorHandlerExtR(options, &keep_error, &last_error);
	TIFFOpenOptionsSetWarningHandlerExtR(options, &log_warning, nullptr);
	auto *handle = TIFFOpenExt(path.c_str(), to_libtiff_mode(mode), options);
	TIFFOpenOptionsFree(options);

	if (handle != nullptr)
	{
		return handle;
	}

	if (mode == tiff_file_mode::read && is_readable(path))
	{
		throw image_format_error(
			path + ": tiff_file: The file can not be opened as a TIFF "
			"file: " + last_error
		);
	}

	throw image_file_error(
		path + ": tiff_file: The file can not be " +
		(mode == tiff_file_mode::read ? "opened: " : "created: ") +
		last_error
	);
}

} // anonymous namespace

tiff_file::tiff_file(const std::string &path, tiff_file_mode mode)
	: m_path(path)
	, m_last_error()
	, m_handle(open_handle(m_path, mode, m_last_error))
{
}

tiff_file::~tiff_file()
{
	TIFFClose(m_handle);
}

std::size_t tiff_file::get_page_count()
{
	return TIFFNumberOfDirectories(m_handle);
}

void tiff_file::select_page(std::size_t page)
{
	if (TIFFSetDirectory(m_handle, static_cast<tdir_t>(page)) == 0)
	{
		throw image_format_error(
			m_path + ": tiff_file: The page can not be selected: " +
			m_last_error
		);
	}
}

tiff_page_layout tiff_file::get_page_layout()
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	if (TIFFGetField(m_handle, TIFFTAG_IMAGEWIDTH, &width) == 0 ||
		TIFFGetField(m_handle, TIFFTAG_IMAGELENGTH, &height) == 0 ||
		width == 0 ||
		height == 0)
	{
		throw image_format_error(
			m_path + ": tiff_file: The page does not state its size."
		);
	}

	std::uint16_t samples_per_pixel = 1;
	TIFFGetFieldDefaulted(
		m_handle, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
	if (samples_per_pixel != 1)
	{
		throw image_format_error(
			m_path + ": tiff_file: The page holds more than one sample per "
			"pixel."
		);
	}

	std::uint16_t bits_per_sample = 1;
	std::uint16_t sample_format = SAMPLEFORMAT_UINT;
	TIFFGetFieldDefaulted(m_handle, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
	TIFFGetFieldDefaulted(m_handle, TIFFTAG_SAMPLEFORMAT, &sample_format);
	const auto data_type = get_data_type(bits_per_sample, sample_format);
	if (data_type == numerical_type::unknown)
	{
		throw image_format_error(
			m_path + ": tiff_file: The samples of the page have no data "
			"type this format transfers."
		);
	}

	std::uint16_t compression = COMPRESSION_NONE;
	TIFFGetFieldDefaulted(m_handle, TIFFTAG_COMPRESSION, &compression);
	if (TIFFIsCODECConfigured(compression) == 0)
	{
		throw image_format_error(
			m_path + ": tiff_file: The page is compressed with a scheme "
			"that can not be decoded."
		);
	}

	if (TIFFIsTiled(m_handle) != 0)
	{
		std::uint32_t tile_width = 0;
		std::uint32_t tile_height = 0;
		if (TIFFGetField(m_handle, TIFFTAG_TILEWIDTH, &tile_width) == 0 ||
			TIFFGetField(m_handle, TIFFTAG_TILELENGTH, &tile_height) == 0 ||
			tile_width == 0 ||
			tile_height == 0)
		{
			throw image_format_error(
				m_path + ": tiff_file: The page does not state the size of "
				"its tiles."
			);
		}

		return tiff_page_layout::make_tiled(
			width, height, data_type, tile_width, tile_height);
	}

	std::uint32_t rows_per_strip = 0;
	TIFFGetFieldDefaulted(m_handle, TIFFTAG_ROWSPERSTRIP, &rows_per_strip);
	if (rows_per_strip == 0)
	{
		throw image_format_error(
			m_path + ": tiff_file: The page states strips of no rows."
		);
	}

	return tiff_page_layout::make_striped(
		width, height, data_type, rows_per_strip);
}

void tiff_file::read_block(std::size_t block, span<byte> destination)
{
	const auto index = static_cast<std::uint32_t>(block);
	const auto size = static_cast<tmsize_t>(destination.size());
	const auto decoded = TIFFIsTiled(m_handle) != 0
		? TIFFReadEncodedTile(m_handle, index, destination.data(), size)
		: TIFFReadEncodedStrip(m_handle, index, destination.data(), size);

	if (decoded < 0)
	{
		throw image_format_error(
			m_path + ": tiff_file: A block of the page can not be decoded: " +
			m_last_error
		);
	}

	if (decoded != size)
	{
		throw image_format_error(
			m_path + ": tiff_file: A block of the page does not hold the "
			"samples its page states."
		);
	}
}

void tiff_file::write_page(
	const tiff_page_layout &layout,
	tiff_compression compression,
	span<const byte> samples
)
{
	if (layout.is_tiled())
	{
		throw std::invalid_argument(
			"tiff_file: A page is only written cut into strips."
		);
	}

	const auto data_type = layout.get_data_type();
	const auto row_size = layout.get_width() * get_size(data_type);
	if (samples.size() != layout.get_height() * row_size)
	{
		throw std::invalid_argument(
			"tiff_file: The samples do not have the size of the page."
		);
	}

	if (!is_supported(data_type))
	{
		throw unsupported_operation_error(
			"tiff_file: The TIFF format has no sample this format transfers "
			"for the data type of the page."
		);
	}

	const auto bits_per_sample = get_bits_per_sample(data_type);
	const auto sample_format = get_sample_format(data_type);

	bool written =
		TIFFSetField(m_handle, TIFFTAG_IMAGEWIDTH,
			static_cast<std::uint32_t>(layout.get_width())) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_IMAGELENGTH,
			static_cast<std::uint32_t>(layout.get_height())) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_BITSPERSAMPLE, bits_per_sample) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_SAMPLESPERPIXEL, 1) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_SAMPLEFORMAT, sample_format) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_PHOTOMETRIC,
			PHOTOMETRIC_MINISBLACK) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_PLANARCONFIG,
			PLANARCONFIG_CONTIG) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_ORIENTATION,
			ORIENTATION_TOPLEFT) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_COMPRESSION,
			to_libtiff_compression(compression)) != 0 &&
		TIFFSetField(m_handle, TIFFTAG_ROWSPERSTRIP,
			static_cast<std::uint32_t>(layout.get_block_height())) != 0;

	const auto block_count = layout.get_block_count();
	std::size_t offset = 0;
	for (std::size_t block = 0; written && block < block_count; ++block)
	{
		const auto size = layout.get_block_size(block);

		// libtiff takes the samples as writable because it swaps them in
		// place when the file is in the other byte order. A file being
		// created is in that of the host, so they are left as they are.
		auto *strip = const_cast<byte*>(samples.data() + offset);
		written = TIFFWriteEncodedStrip(
			m_handle,
			static_cast<std::uint32_t>(block),
			strip,
			static_cast<tmsize_t>(size)
		) == static_cast<tmsize_t>(size);

		offset += size;
	}

	written = written && TIFFWriteDirectory(m_handle) != 0;

	if (!written)
	{
		throw image_file_error(
			m_path + ": tiff_file: The page can not be written: " +
			m_last_error
		);
	}
}

} // namespace tiff
} // namespace em
} // namespace rexlib
