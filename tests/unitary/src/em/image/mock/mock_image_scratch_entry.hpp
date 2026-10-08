// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_scratch_entry.hpp>

#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_scratch_entry final
	: public image_scratch_entry
{
public:
	mock_image_scratch_entry() = default;

	MAKE_CONST_MOCK2(
		read,
		image_transfer_plan(
			array_ref destination,
			const image_transfer_plan &regions
		),
		override
	);

	MAKE_MOCK2(
		store,
		void(const image_reader &file, const image_transfer_plan &regions),
		override
	);
};

} // namespace em
} // namespace rexlib
