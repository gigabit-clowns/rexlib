// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/parameters/sequence_length.hpp>

namespace rexlib
{
namespace ops
{

sequence_length::sequence_length(std::size_t count) noexcept
	: m_count(count)
{
}

std::size_t sequence_length::get_count() const noexcept
{
	return m_count;
}

} // namespace ops
} // namespace rexlib
