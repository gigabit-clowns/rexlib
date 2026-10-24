// SPDX-License-Identifier: GPL-3.0-only

#include "operand_data_types.hpp"

namespace rexlib
{
namespace cpu
{

template <std::size_t Count>
void extract_data_types(
	std::array<numerical_type, Count> &types,
	span<const operand_descriptor> descriptors
) noexcept
{
	for (std::size_t i = 0; i < Count; ++i)
	{
		types[i] = descriptors[i].get_data_type();
	}
}

} // namespace cpu
} // namespace rexlib
