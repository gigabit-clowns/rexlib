// SPDX-License-Identifier: GPL-3.0-only

#include "count_of.hpp"

namespace rexlib
{
namespace ops
{

template <typename Parameters>
inline
std::size_t count_of::get(const Parameters &parameters) noexcept
{
	return parameters.get_count();
}

} // namespace ops
} // namespace rexlib
