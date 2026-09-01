// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_writer.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_writer final
	: public image_writer
{
public:
	mock_image_writer() = default;

	MAKE_CONST_MOCK0(
		get_descriptor,
		const array_descriptor&(),
		noexcept override
	);

	MAKE_MOCK2(
		write_region,
		void(span<const std::size_t> offset, const_array_ref source),
		override
	);

	MAKE_MOCK0(flush, void(), override);
};

} // namespace em
} // namespace rexlib
