// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_transaction_plan.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

image_transaction_plan::image_transaction_plan() noexcept = default;

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

void image_transaction_plan::reset(
	span<const std::size_t> extents,
	std::size_t file_rank,
	std::size_t array_rank
)
{
	if (extents.size() > file_rank)
	{
		throw std::invalid_argument(
			"image_transaction_plan::reset: The regions do not fit in the "
			"rank of the files."
		);
	}

	if (extents.size() > array_rank)
	{
		throw std::invalid_argument(
			"image_transaction_plan::reset: The regions do not fit in the "
			"rank of the array."
		);
	}

	m_extents.assign(extents.begin(), extents.end());
	m_file_offsets.reset(file_rank);
	m_array_offsets.reset(array_rank);
	m_files.clear();
}

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

std::size_t image_transaction_plan::get_size() const noexcept
{
	return m_files.get_size();
}

std::size_t image_transaction_plan::get_rank() const noexcept
{
	return m_extents.size();
}

std::size_t image_transaction_plan::get_file_rank() const noexcept
{
	return m_file_offsets.get_rank();
}

std::size_t image_transaction_plan::get_array_rank() const noexcept
{
	return m_array_offsets.get_rank();
}

span<const std::size_t> image_transaction_plan::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
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
image_transaction_plan::get_region_file(std::size_t region) const noexcept
{
	return m_files.get_path_index(region);
}

span<const std::size_t>
image_transaction_plan::get_file_offset(std::size_t region) const noexcept
{
	return m_file_offsets.get(region);
}

span<const std::size_t>
image_transaction_plan::get_array_offset(std::size_t region) const noexcept
{
	return m_array_offsets.get(region);
}

} // namespace em
} // namespace rexlib
