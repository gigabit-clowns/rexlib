// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_reader_provider.hpp>

#include <rexlib/em/image/image_reader.hpp>

namespace rexlib
{
namespace em
{

image_reader_provider::image_reader_provider() noexcept = default;
image_reader_provider::~image_reader_provider() = default;

image_descriptor query_descriptor(
	image_reader_provider &readers,
	const std::string &path
)
{
	return readers.acquire(path)->get_descriptor();
}

} // namespace em
} // namespace rexlib
