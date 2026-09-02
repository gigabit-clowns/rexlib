// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_reader.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/image_access_traits.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_reader final
	: public image_reader
{
public:
	mock_image_reader() = default;

	MAKE_CONST_MOCK0(
		get_descriptor,
		const array_descriptor&(),
		noexcept override
	);

	MAKE_CONST_MOCK0(
		get_metadata,
		const image_metadata&(),
		noexcept override
	);

	MAKE_CONST_MOCK0(
		get_access_traits,
		const image_access_traits&(),
		noexcept override
	);

	MAKE_CONST_MOCK2(
		read,
		void(array_ref destination, const image_transfer_plan &regions),
		override
	);
};

} // namespace em
} // namespace rexlib
