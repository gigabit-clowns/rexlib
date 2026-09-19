// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_reader_provider.hpp>

#include "image_reader_extents.hpp"

#include <rexlib/em/image/image_reader.hpp>

namespace rexlib
{
namespace em
{

image_reader_provider::image_reader_provider() noexcept = default;
image_reader_provider::~image_reader_provider() = default;

std::vector<std::size_t> query_extents(
	image_reader_provider &readers,
	const std::string &path
)
{
	return copy_extents(*readers.acquire(path));
}

std::vector<std::size_t> query_core_extents(
	image_reader_provider &readers,
	const std::string &path
)
{
	return copy_core_extents(*readers.acquire(path));
}

} // namespace em
} // namespace rexlib
