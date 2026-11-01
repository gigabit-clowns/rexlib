// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/random/split_key_operation.hpp>

namespace rexlib
{
namespace ops
{

split_key_operation::split_key_operation(std::size_t count)
	: parametric_operation(count)
{
}

std::size_t split_key_operation::get_count() const noexcept
{
	return get_parameters().get_count();
}

} // namespace ops
} // namespace rexlib
