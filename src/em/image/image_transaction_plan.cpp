// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_transaction_plan.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

image_transaction_plan::image_transaction_plan(image_transfer_shape shape)
	: m_shape(std::move(shape))
	, m_file_offsets(m_shape.get_file_rank())
	, m_array_offsets(m_shape.get_array_rank())
{
}

image_transaction_plan::image_transaction_plan(
	const image_transaction_plan &other
) = default;
image_transaction_plan::image_transaction_plan(
	image_transaction_plan &&other
) noexcept = default;
image_transaction_plan::~image_transaction_plan() = default;

image_transaction_plan& image_transaction_plan::operator=(
	const image_transaction_plan &other
) = default;
image_transaction_plan& image_transaction_plan::operator=(
	image_transaction_plan &&other
) noexcept = default;

std::size_t image_transaction_plan::add_file(std::string path)
{
	return m_files.intern(std::move(path));
}

void image_transaction_plan::add(
	std::size_t file_index,
	span<const std::size_t> file_offset,
	span<const std::size_t> array_offset
)
{
	if (file_index >= m_files.get_path_count())
	{
		throw std::out_of_range(
			"image_transaction_plan::add: The file index names no file of "
			"this plan."
		);
	}

	if (file_offset.size() != m_file_offsets.get_rank())
	{
		throw std::invalid_argument(
			"image_transaction_plan::add: The file offset does not have the "
			"rank of the files."
		);
	}

	if (array_offset.size() != m_array_offsets.get_rank())
	{
		throw std::invalid_argument(
			"image_transaction_plan::add: The array offset does not have "
			"the rank of the array."
		);
	}

	m_file_offsets.add(file_offset);
	m_array_offsets.add(array_offset);
	m_files.append(file_index);
}

void image_transaction_plan::clear() noexcept
{
	m_files.clear();
	m_file_offsets.clear();
	m_array_offsets.clear();
}

void image_transaction_plan::reserve(
	std::size_t files,
	std::size_t regions
)
{
	m_files.reserve(files, regions);
	m_file_offsets.reserve(regions);
	m_array_offsets.reserve(regions);
}

std::size_t image_transaction_plan::get_region_count() const noexcept
{
	return m_files.get_entry_count();
}

const image_transfer_shape&
image_transaction_plan::get_shape() const noexcept
{
	return m_shape;
}

std::size_t image_transaction_plan::get_file_count() const noexcept
{
	return m_files.get_path_count();
}

const std::string&
image_transaction_plan::get_file(std::size_t file_index) const noexcept
{
	return m_files.get_path(file_index);
}

std::size_t
image_transaction_plan::get_region_file(std::size_t region_index) const noexcept
{
	return m_files.get_path_index(region_index);
}

span<const std::size_t>
image_transaction_plan::get_file_offset(std::size_t region_index) const noexcept
{
	return m_file_offsets.get(region_index);
}

span<const std::size_t>
image_transaction_plan::get_array_offset(
	std::size_t region_index
) const noexcept
{
	return m_array_offsets.get(region_index);
}

} // namespace em
} // namespace rexlib
