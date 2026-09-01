// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_open_options.hpp>

namespace rexlib
{
namespace em
{

image_open_options::image_open_options() noexcept
	: m_dataset_index(0)
	, m_access_hint(image_access_hint::none)
{
}

void image_open_options::set_dataset_index(std::size_t index) noexcept
{
	m_dataset_index = index;
}

std::size_t image_open_options::get_dataset_index() const noexcept
{
	return m_dataset_index;
}

void image_open_options::set_access_hint(image_access_hint hint) noexcept
{
	m_access_hint = hint;
}

image_access_hint image_open_options::get_access_hint() const noexcept
{
	return m_access_hint;
}

} // namespace em
} // namespace rexlib
