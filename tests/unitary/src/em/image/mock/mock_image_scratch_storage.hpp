// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_scratch_storage.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_scratch_storage final
	: public image_scratch_storage
{
public:
	mock_image_scratch_storage() = default;

	MAKE_MOCK0(get_data, byte*(), noexcept override);
	MAKE_CONST_MOCK0(get_data, const byte*(), noexcept override);
	MAKE_CONST_MOCK0(get_size, std::size_t(), noexcept override);
};

} // namespace em
} // namespace rexlib
