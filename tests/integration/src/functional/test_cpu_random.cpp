// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/random.hpp>

#include "fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/layout/dynamic_subscript.hpp>
#include <rexlib/core/layout/slice.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/functional/view.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace rexlib;

namespace
{

using extent_list_type = std::vector<std::size_t>;
using key_list_type = std::vector<std::uint64_t>;

array make_keys(
	const execution_context &context,
	const extent_list_type &extents,
	const key_list_type &values
)
{
	auto result = empty(
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::uint64
		),
		memory_resource_affinity::device,
		context
	);

	auto *data =
		static_cast<std::uint64_t*>(result.get_storage()->get_host_ptr());
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		data[i] = values[i];
	}
	return result;
}

extent_list_type extents_of(const array &value)
{
	extent_list_type result;
	value.get_descriptor().get_layout().get_extents(result);
	return result;
}

} // anonymous namespace

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"make_key holds the seed in a uint64 array without axes",
	"[random][cpu]"
)
{
	const auto key = make_key(0x0123456789abcdefu, context);

	CHECK( extents_of(key).empty() );
	CHECK(
		key.get_descriptor().get_data_type() == numerical_type::uint64
	);
	CHECK( read_host<std::uint64_t>(key, 1)[0] == 0x0123456789abcdefu );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"fold_in_key derives the known keys of a key and some values",
	"[random][cpu]"
)
{
	const auto key = make_key(42, context);
	const auto data = make_keys(context, { 3 }, { 0, 1, 2 });

	const auto result = fold_in_key(key, data, context);

	CHECK( extents_of(result) == extent_list_type{ 3 } );
	CHECK(
		read_host<std::uint64_t>(result, 3) == key_list_type{
			0x9807e02220439b04u,
			0x5a936ff51f858d22u,
			0x97af2590207eb3d4u
		}
	);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"fold_in_key broadcasts the keys against the values",
	"[random][cpu]"
)
{
	const auto keys = make_keys(context, { 2, 1 }, { 42, 7 });
	const auto data = make_keys(context, { 3 }, { 0, 1, 3 });

	const auto result = fold_in_key(keys, data, context);

	CHECK( extents_of(result) == extent_list_type{ 2, 3 } );
	const auto values = read_host<std::uint64_t>(result, 6);
	CHECK( values[0] == 0x9807e02220439b04u );
	CHECK( values[1] == 0x5a936ff51f858d22u );
	CHECK( values[5] == 0x89ec9187b1dfe290u );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"split_key derives the keys fold_in_key derives from each key and the "
	"positions along a new last axis",
	"[random][cpu]"
)
{
	const auto keys = make_keys(context, { 2 }, { 42, 7 });
	const auto positions = make_keys(context, { 1, 4 }, { 0, 1, 2, 3 });
	const auto columns = make_keys(context, { 2, 1 }, { 42, 7 });

	const auto result = split_key(keys, 4, context);
	const auto expected = fold_in_key(columns, positions, context);

	CHECK( extents_of(result) == extent_list_type{ 2, 4 } );
	CHECK(
		read_host<std::uint64_t>(result, 8) ==
		read_host<std::uint64_t>(expected, 8)
	);
	CHECK( read_host<std::uint64_t>(result, 8)[2] == 0x97af2590207eb3d4u );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"split_key splits a key without axes into a sequence",
	"[random][cpu]"
)
{
	const auto key = make_key(42, context);

	const auto result = split_key(key, 2, context);

	CHECK( extents_of(result) == extent_list_type{ 2 } );
	CHECK(
		read_host<std::uint64_t>(result, 2) == key_list_type{
			0x9807e02220439b04u,
			0x5a936ff51f858d22u
		}
	);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"split_key reads strided and reversed views",
	"[random][cpu]"
)
{
	auto base = make_keys(context, { 2, 6 }, {
		1, 2, 3, 4, 5, 6,
		7, 8, 9, 10, 11, 12
	});
	const std::array<dynamic_subscript, 2> subscripts = {{
		slice(1, 2, -1),
		slice(0, 3, 2)
	}};
	const auto view = subarray(base, make_span(subscripts));
	const auto contiguous = copy(view, context);

	const auto from_view = split_key(view, 3, context);
	const auto from_copy = split_key(contiguous, 3, context);

	CHECK( extents_of(from_view) == extent_list_type{ 2, 3, 3 } );
	CHECK(
		read_host<std::uint64_t>(from_view, 18) ==
		read_host<std::uint64_t>(from_copy, 18)
	);
}
