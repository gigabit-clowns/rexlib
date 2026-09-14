// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_file_mapping.hpp"

#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <cstddef>
#include <fstream>

namespace rexlib
{
namespace em
{
namespace mrc
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

} // anonymous namespace

mrc_file_mapping::mrc_file_mapping(
	const std::string &path,
	access_flags access
)
{
	const auto mode = to_mode(access);

	try
	{
		m_mapping = std::make_unique<boost::interprocess::file_mapping>(
			path.c_str(), mode
		);
		m_region = std::make_unique<boost::interprocess::mapped_region>(
			*m_mapping, mode
		);
	}
	catch (const boost::interprocess::interprocess_exception &error)
	{
		throw image_format_error(
			"mrc_file_mapping: The file could not be mapped: " +
			std::string(error.what())
		);
	}

	if (m_region->get_size() == 0)
	{
		throw image_format_error(
			"mrc_file_mapping: The file is empty."
		);
	}
}

mrc_file_mapping::mrc_file_mapping(
	mrc_file_mapping &&other
) noexcept = default;

mrc_file_mapping::~mrc_file_mapping() = default;

mrc_file_mapping&
mrc_file_mapping::operator=(mrc_file_mapping &&other) noexcept = default;

byte* mrc_file_mapping::get_data() const noexcept
{
	return static_cast<byte*>(m_region->get_address());
}

std::size_t mrc_file_mapping::get_size() const noexcept
{
	return m_region->get_size();
}

void mrc_file_mapping::prefetch(
	span<const memory_range> ranges
) const noexcept
{
	prefetch_pages(get_data(), ranges);
}

void mrc_file_mapping::flush()
{
	if (!m_region->flush())
	{
		throw image_format_error(
			"mrc_file_mapping: The mapping could not be flushed."
		);
	}
}

void create_file(const std::string &path, std::size_t size)
{
	if (size == 0)
	{
		throw image_format_error(
			"mrc::create_file: A file of no bytes can not be mapped."
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
			throw image_format_error(
				"mrc::create_file: The file could not be created."
			);
		}
	}

	boost::system::error_code code;
	boost::filesystem::resize_file(path, size, code);
	if (code)
	{
		throw image_format_error(
			"mrc::create_file: The file could not be sized: " +
			code.message()
		);
	}
}

} // namespace mrc
} // namespace em
} // namespace rexlib
