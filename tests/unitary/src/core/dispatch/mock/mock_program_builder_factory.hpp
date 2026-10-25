// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder.hpp>

#include <memory>
#include <trompeloeil.hpp>

namespace rexlib
{

// A registry keeps its factories as capture-less functions, which reach no
// object of their own. create is one, forwarding to the single instance of
// this mock, the way trompeloeil mocks a free function.
class mock_program_builder_factory final
{
public:
	MAKE_MOCK0(make, std::unique_ptr<program_builder>());

	static mock_program_builder_factory& get_instance()
	{
		static mock_program_builder_factory instance;
		return instance;
	}

	static std::unique_ptr<program_builder> create()
	{
		return get_instance().make();
	}

private:
	mock_program_builder_factory() = default;
};

} // namespace rexlib
