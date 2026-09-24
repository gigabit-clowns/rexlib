// SPDX-License-Identifier: GPL-3.0-only

#include "image_file_mapping.hpp"

#include <rexlib/core/platform/attributes.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>

#include <boost/filesystem/operations.hpp>

#include <cstddef>
#include <fstream>

namespace rexlib
{
namespace em
{

namespace
{

boost::interprocess::mode_t to_mode(access_flags access)
{
	if (access.contains(access_flag_bits::write))
	{
		return boost::interprocess::read_write;
	}

	return boost::interprocess::read_only;
}

REXLIB_NORETURN
void throw_unmappable(
	const boost::interprocess::interprocess_exception &error
)
{
	throw image_file_error(
		"image_file_mapping: The file could not be mapped: " +
		std::string(error.what())
	);
}

boost::interprocess::file_mapping open_mapping(
	const std::string &path,
	access_flags access
)
{
	try
	{
		return boost::interprocess::file_mapping(path.c_str(), to_mode(access));
	}
	catch (const boost::interprocess::interprocess_exception &error)
	{
		throw_unmappable(error);
	}
}

boost::interprocess::mapped_region map_region(
	const boost::interprocess::file_mapping &mapping,
	access_flags access
)
{
	try
	{
		return boost::interprocess::mapped_region(mapping, to_mode(access));
	}
	catch (const boost::interprocess::interprocess_exception &error)
	{
		throw_unmappable(error);
	}
}

} // anonymous namespace

image_file_mapping::image_file_mapping(
	const std::string &path,
	access_flags access
)
	: m_mapping(open_mapping(path, access))
	, m_region(map_region(m_mapping, access))
{
	if (m_region.get_size() == 0)
	{
		throw image_file_error(
			"image_file_mapping: The file is empty."
		);
	}
}

byte* image_file_mapping::get_data() const noexcept
{
	return static_cast<byte*>(m_region.get_address());
}

std::size_t image_file_mapping::get_size() const noexcept
{
	return m_region.get_size();
}

void image_file_mapping::flush()
{
	if (!m_region.flush())
	{
		throw image_file_error(
			"image_file_mapping: The mapping could not be flushed."
		);
	}
}

void create_image_file(const std::string &path, std::size_t size)
{
	if (size == 0)
	{
		throw image_file_error(
			"create_image_file: A file of no bytes can not be mapped."
		);
	}

	{
		std::filebuf file;
		const auto opened = file.open(
			path,
			std::ios_base::in | std::ios_base::out |
			std::ios_base::trunc | std::ios_base::binary
		);
		if (opened == nullptr)
		{
			throw image_file_error(
				"create_image_file: The file could not be created."
			);
		}
	}

	boost::system::error_code code;
	boost::filesystem::resize_file(path, size, code);
	if (code)
	{
		throw image_file_error(
			"create_image_file: The file could not be sized: " +
			code.message()
		);
	}
}

} // namespace em
} // namespace rexlib
