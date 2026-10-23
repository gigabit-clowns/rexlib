// SPDX-License-Identifier: GPL-3.0-only

#include "keep_dimensions_of.hpp"

namespace rexlib
{
namespace ops
{

template <typename Parameters>
inline
bool keep_dimensions_of::get(const Parameters &parameters) noexcept
{
	return parameters.get_keep_dimensions();
}

} // namespace ops
} // namespace rexlib
