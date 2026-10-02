// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_transfer_sanitizer.hpp>

#include <memory>

namespace rexlib
{
namespace em
{

/**
 * @brief A sanitizer that clips every region to both of its sides.
 *
 * A region is cut down to the intersection of what the file holds at its
 * file offset with what the array holds at its array offset, up to the
 * extents of the plan. The intersection begins where the region begins on
 * either side, so clipping only ever shortens the extents and never moves an
 * offset. A region reaching past both sides is shortened by whichever runs
 * out first.
 *
 * A plan carries one set of extents for every region it holds, so regions
 * clipping to different extents cannot share one. They are grouped by the
 * extents they clip to and one plan is answered per group, in the order the
 * groups were first met. Regions of one group keep the order they had.
 *
 * A region clipping to nothing along any axis is dropped. That includes one
 * reaching past a side along an axis the extents of the plan do not cover,
 * which spans a single position and so cannot be shortened.
 */
class REXLIB_API clipping_image_transfer_sanitizer final
	: public image_transfer_sanitizer
{
public:
	std::vector<image_transfer_plan> sanitize(
		const image_transfer_plan &regions,
		span<const std::size_t> file_extents,
		span<const std::size_t> array_extents
	) const override;

	/**
	 * @brief Get the instance every use shares.
	 *
	 * @return const std::shared_ptr<const clipping_image_transfer_sanitizer>&
	 * The instance, never null.
	 */
	static const std::shared_ptr<const clipping_image_transfer_sanitizer>&
	get_shared();
};

} // namespace em
} // namespace rexlib
