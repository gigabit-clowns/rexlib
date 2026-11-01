// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

namespace rexlib
{
namespace em
{

insert_weighted_slices_operation::insert_weighted_slices_operation(
	const gridding_window &window
) noexcept
	: m_window(window)
{
}

const gridding_window&
insert_weighted_slices_operation::get_window() const noexcept
{
	return m_window;
}

} // namespace em
} // namespace rexlib
