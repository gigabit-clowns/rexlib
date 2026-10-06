// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/host_image_scratch_storage.hpp>

#include <rexlib/core/memory/align.hpp>
#include <rexlib/core/memory/aligned_alloc.hpp>
#include <rexlib/core/system/host.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>

#include <new>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

byte* allocate(std::size_t size)
{
	if (size == 0)
	{
		throw std::invalid_argument(
			"create_host_image_scratch_storage: The size must not be zero."
		);
	}

	const auto alignment = get_page_size();
	auto *data = aligned_alloc(align_ceil(size, alignment), alignment);
	if (data == nullptr)
	{
		throw std::bad_alloc();
	}

	return static_cast<byte*>(data);
}

class host_image_scratch_storage final
	: public image_scratch_storage
{
public:
	explicit host_image_scratch_storage(std::size_t size)
		: m_data(allocate(size))
		, m_size(size)
	{
	}

	~host_image_scratch_storage() override
	{
		aligned_free(m_data);
	}

	byte* get_data() noexcept override
	{
		return m_data;
	}

	const byte* get_data() const noexcept override
	{
		return m_data;
	}

	std::size_t get_size() const noexcept override
	{
		return m_size;
	}

private:
	byte *m_data;
	std::size_t m_size;
};

} // anonymous namespace

std::shared_ptr<image_scratch_storage>
create_host_image_scratch_storage(std::size_t size)
{
	return std::make_shared<host_image_scratch_storage>(size);
}

} // namespace em
} // namespace rexlib
