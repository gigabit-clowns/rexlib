// SPDX-License-Identifier: GPL-3.0-only

#include "axes_of.hpp"

namespace rexlib
{
namespace ops
{

template <typename Parameters>
inline
span<const std::size_t> axes_of::get(const Parameters &parameters) noexcept
{
	return parameters.get_axes();
}

} // namespace ops
} // namespace rexlib
