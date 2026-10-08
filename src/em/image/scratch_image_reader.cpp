// SPDX-License-Identifier: GPL-3.0-only

#include "scratch_image_reader.hpp"

#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <utility>

namespace rexlib
{
namespace em
{

scratch_image_reader::scratch_image_reader(
	std::shared_ptr<image_scratch_entry> entry,
	std::shared_ptr<const image_reader> file
)
	: m_entry(std::move(entry))
	, m_file(std::move(file))
{
}

scratch_image_reader::~scratch_image_reader() = default;

const image_descriptor& scratch_image_reader::get_descriptor() const noexcept
{
	REXLIB_ASSERT(m_file);
	return m_file->get_descriptor();
}

const image_metadata& scratch_image_reader::get_metadata() const noexcept
{
	REXLIB_ASSERT(m_file);
	return m_file->get_metadata();
}

void scratch_image_reader::read(
	array_ref destination,
	const image_transfer_plan &regions
) const
{
	REXLIB_ASSERT(m_entry);
	const auto missing = m_entry->read(destination, regions);
	if (missing.get_region_count() == 0)
	{
		return;
	}

	REXLIB_ASSERT(m_file);
	m_entry->store(*m_file, missing);

	const auto unheld = m_entry->read(destination, missing);
	if (unheld.get_region_count() == 0)
	{
		return;
	}
	
	m_file->read(destination, unheld);
}

} // namespace em
} // namespace rexlib
