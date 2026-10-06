// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/mapped_file_image_scratch_storage.hpp>

#include <em/image/formats/memory_mapping/image_file_mapping.hpp>

#include <rexlib/core/system/access_flags.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>

namespace rexlib
{
namespace em
{

namespace
{

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
	create_image_file(path, size);

	return std::make_shared<mapped_file_image_scratch_storage>(path);
}

std::shared_ptr<image_scratch_storage>
open_mapped_file_image_scratch_storage(const std::string &path)
{
	return std::make_shared<mapped_file_image_scratch_storage>(path);
}

} // namespace em
} // namespace rexlib
