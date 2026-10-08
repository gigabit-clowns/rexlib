// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/mapped_file_image_scratch_storage.hpp>

#include <em/image/formats/memory_mapping/image_file_mapping.hpp>

#include <rexlib/core/system/access_flags.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>

#include <boost/filesystem/operations.hpp>

#include <fstream>

namespace rexlib
{
namespace em
{

namespace
{

// Makes sure that a file of a size is at a path. A file that is already
// there is never emptied, since another storage may have it mapped.
void lay_out_file(const std::string &path, std::size_t size)
{
	if (size == 0)
	{
		throw image_file_error(
			path + ": create_mapped_file_image_scratch_storage: A file of "
			"no bytes can not be mapped."
		);
	}

	{
		// Opening a file to append creates it if it is missing, and leaves
		// it as it is otherwise.
		std::filebuf file;
		const auto opened = file.open(
			path,
			std::ios_base::out | std::ios_base::app | std::ios_base::binary
		);
		if (opened == nullptr)
		{
			throw image_file_error(
				path + ": create_mapped_file_image_scratch_storage: The "
				"file could not be created."
			);
		}
	}

	boost::system::error_code error;
	boost::filesystem::resize_file(path, size, error);
	if (error)
	{
		throw image_file_error(
			path + ": create_mapped_file_image_scratch_storage: The file "
			"could not be sized: " + error.message()
		);
	}
}

class mapped_file_image_scratch_storage final
	: public image_scratch_storage
{
public:
	explicit mapped_file_image_scratch_storage(const std::string &path)
		: m_mapping(path, read_write)
	{
	}

	~mapped_file_image_scratch_storage() override = default;

	byte* get_data() noexcept override
	{
		return m_mapping.get_data();
	}

	const byte* get_data() const noexcept override
	{
		return m_mapping.get_data();
	}

	std::size_t get_size() const noexcept override
	{
		return m_mapping.get_size();
	}

private:
	image_file_mapping m_mapping;
};

} // anonymous namespace

std::shared_ptr<image_scratch_storage>
create_mapped_file_image_scratch_storage(
	const std::string &path,
	std::size_t size
)
{
	lay_out_file(path, size);

	return std::make_shared<mapped_file_image_scratch_storage>(path);
}

} // namespace em
} // namespace rexlib
