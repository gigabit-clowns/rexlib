// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/parameters/signal_extent.hpp>

namespace rexlib
{
namespace ops
{

signal_extent::signal_extent(std::size_t extent) noexcept
	: m_extent(extent)
{
}

std::size_t signal_extent::get_extent() const noexcept
{
	return m_extent;
}

} // namespace ops
} // namespace rexlib
