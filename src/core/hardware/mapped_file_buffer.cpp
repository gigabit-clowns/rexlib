// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/mapped_file_buffer.hpp>

#include <rexlib/core/exceptions/file_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <fstream>
#include <stdexcept>
#include <utility>

namespace rexlib
{

namespace
{

void create_file(const std::string &path, std::size_t size)
{
	{
		std::filebuf file;
		const auto opened = file.open(
			path,
			std::ios_base::in | std::ios_base::out |
			std::ios_base::trunc | std::ios_base::binary
		);
		if (opened == nullptr)
		{
			throw file_error(
				path + ": create_mapped_file_buffer: The file could not be "
				"created."
			);
		}
	}

	boost::system::error_code code;
	boost::filesystem::resize_file(path, size, code);
	if (code)
	{
		throw file_error(
			path + ": create_mapped_file_buffer: The file could not be "
			"sized: " + code.message()
		);
	}
}

boost::interprocess::mapped_region map_file(const std::string &path)
{
	try
	{
		const boost::interprocess::file_mapping file(
			path.c_str(),
			boost::interprocess::read_write
		);

		return boost::interprocess::mapped_region(
			file,
			boost::interprocess::read_write
		);
	}
	catch (const boost::interprocess::interprocess_exception &error)
	{
		throw file_error(
			path + ": create_mapped_file_buffer: The file could not be "
			"mapped: " + error.what()
		);
	}
}

class mapped_file_buffer final
	: public buffer
{
public:
	explicit mapped_file_buffer(boost::interprocess::mapped_region region)
		: m_region(std::move(region))
	{
	}

	~mapped_file_buffer() override = default;

	void* get_host_ptr() noexcept override
	{
		return m_region.get_address();
	}

	const void* get_host_ptr() const noexcept override
	{
		return m_region.get_address();
	}

	std::size_t get_size() const noexcept override
	{
		return m_region.get_size();
	}

	const memory_resource& get_memory_resource() const noexcept override
	{
		return get_host_memory_resource();
	}

private:
	boost::interprocess::mapped_region m_region;
};

} // anonymous namespace

std::shared_ptr<buffer> create_mapped_file_buffer(
	const std::string &path,
	std::size_t size
)
{
	if (size == 0)
	{
		throw std::invalid_argument(
			"create_mapped_file_buffer: A file of no bytes can not be mapped."
		);
	}

	create_file(path, size);

	return std::make_shared<mapped_file_buffer>(map_file(path));
}

} // namespace rexlib
