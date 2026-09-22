// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_reader.hpp>

#include <rexlib/core/platform/assert.hpp>

namespace rexlib
{
namespace em
{

image_reader::image_reader() noexcept = default;
image_reader::~image_reader() = default;

std::vector<std::size_t> copy_extents(const image_reader &reader)
{
	const auto extents = reader.get_extents();
	return std::vector<std::size_t>(extents.begin(), extents.end());
}

std::vector<std::size_t> copy_core_extents(const image_reader &reader)
{
	const auto extents = reader.get_extents();
	const auto core_rank = reader.get_core_rank();

	REXLIB_ASSERT(core_rank <= extents.size());

	return std::vector<std::size_t>(
		extents.end() - static_cast<std::ptrdiff_t>(core_rank),
		extents.end()
	);
}

} // namespace em
} // namespace rexlib
