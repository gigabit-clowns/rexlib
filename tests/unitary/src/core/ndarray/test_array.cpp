// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include "../hardware/mock/mock_buffer.hpp"

#include <memory>
#include <vector>

using namespace rexlib;

TEST_CASE("Default constructed array should have no storage", "[array]")
{
	array arr;
	CHECK( arr.get_storage() == nullptr );
	CHECK( arr.share_storage() == nullptr );
}

TEST_CASE("Default constructed array should have an empty descriptor", "[array]")
{
	array arr;
	CHECK( arr.get_descriptor() == array_descriptor() );
}

TEST_CASE("Constructing an array should store its attributes", "[array]")
{
	const auto data_type = GENERATE(
		numerical_type::int32,
		numerical_type::float64
	);

	const auto extents = GENERATE(
		std::vector<std::size_t>{1},
		std::vector<std::size_t>{20, 50}
	);

	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();

	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const array_descriptor descriptor(layout, data_type);
	array arr(storage, descriptor);

	CHECK( arr.get_storage() == storage.get() );
	CHECK( arr.share_storage() == storage );
	CHECK( arr.get_descriptor() == descriptor );
}	

TEST_CASE("Calling share on an array should return an array with the same content", "[array]")
{
	const auto data_type = GENERATE(
		numerical_type::int32,
		numerical_type::float64
	);

	const auto extents = GENERATE(
		std::vector<std::size_t>{1},
		std::vector<std::size_t>{20, 50}
	);

	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();

	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const array_descriptor descriptor(layout, data_type);
	array arr1(storage, descriptor);
	auto arr2 = arr1.share();
	auto arr3 = static_cast<const array&>(arr2).share();

	CHECK( &arr1.get_descriptor() == &arr2.get_descriptor() );
	CHECK( arr1.get_storage() == arr2.get_storage() );
	CHECK( &arr1.get_descriptor() == &arr3.get_descriptor() );
	CHECK( arr1.get_storage() == arr3.get_storage() );
}

TEST_CASE(
	"Default constructed array should have no access hazard tracker",
	"[array]"
)
{
	const array arr;
	CHECK( arr.get_access_hazard_tracker() == nullptr );
}

TEST_CASE(
	"An array should share its access hazard tracker with its aliases",
	"[array]"
)
{
	const std::vector<std::size_t> extents = {20, 50};
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32
	);

	array arr(std::make_shared<mock_buffer>(), descriptor);
	const auto alias = arr.share();
	const auto const_alias = arr.share_const();

	REQUIRE( arr.get_access_hazard_tracker() != nullptr );
	CHECK(
		alias.get_access_hazard_tracker() ==
		arr.get_access_hazard_tracker()
	);
	CHECK(
		const_alias.get_access_hazard_tracker() ==
		arr.get_access_hazard_tracker()
	);
}

TEST_CASE(
	"Arrays built separately over one buffer should have a tracker each",
	"[array]"
)
{
	const std::vector<std::size_t> extents = {20, 50};
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32
	);

	const auto storage = std::make_shared<mock_buffer>();
	const array first(storage, descriptor);
	const array second(storage, descriptor);

	CHECK(
		first.get_access_hazard_tracker() !=
		second.get_access_hazard_tracker()
	);
}
