// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

// A registry keeps its factories as capture-less functions, which reach no
// object of their own. create is one, forwarding to the single instance of
// this mock per format type, the way trompeloeil mocks a free function.
template <typename Format>
class mock_image_format_factory final
{
public:
	MAKE_MOCK0(make, std::unique_ptr<Format>());

	static mock_image_format_factory& get_instance()
	{
		static mock_image_format_factory instance;
		return instance;
	}

	static std::unique_ptr<Format> create()
	{
		return get_instance().make();
	}

private:
	mock_image_format_factory() = default;
};

} // namespace em
} // namespace rexlib
