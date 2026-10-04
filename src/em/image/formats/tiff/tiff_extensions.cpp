// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_extensions.hpp"

namespace rexlib
{
namespace em
{
namespace tiff
{

bool is_writable_extension(const std::string &extension) noexcept
{
	return extension == ".tif" || extension == ".tiff";
}

} // namespace tiff
} // namespace em
} // namespace rexlib
