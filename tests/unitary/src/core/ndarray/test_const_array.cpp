// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rex/core/ndarray/const_array.hpp>

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>
#include <core/ndarray/array_implementation.hpp>

#include "../hardware/mock/mock_buffer.hpp"

#include <memory>
#include <type_traits>
#include <vector>

using namespace rex;

TEST_CASE("Default constructed const_array should have no storage", "[const_array]")
{
	const_array arr;
	CHECK( arr.get_storage() == nullptr );
	CHECK( arr.share_storage() == nullptr );
}

TEST_CASE("Default constructed const_array should have an empty descriptor", "[const_array]")
{
	const_array arr;
	CHECK( arr.get_descriptor() == array_descriptor() );
}

TEST_CASE("Constructing an const_array should store its attributes", "[const_array]")
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

	const_array view;
	SECTION("from implementation")
	{
		view = const_array(
			std::make_shared<array_implementation>(storage, descriptor)
		);
	}
	SECTION("implicitly from another array")
	{
		view = array(storage, descriptor).share_const();
	}

	CHECK( view.get_storage() == storage.get() );
	CHECK( view.share_storage() == storage );
	CHECK( view.get_descriptor() == descriptor );
}	

TEST_CASE("Calling share on an const_array should return an array with the same content", "[const_array]")
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
	const const_array view1(
		std::make_shared<array_implementation>(storage, descriptor)
	);
	const auto view2 = view1.share();

	CHECK( &view1.get_descriptor() == &view2.get_descriptor() );
	CHECK( view1.get_storage() == view2.get_storage() );
}	

TEST_CASE("const_array should not be copyable nor implicitly convertible", "[const_array]")
{
	CHECK_FALSE( std::is_copy_constructible<const_array>::value );
	CHECK_FALSE( std::is_copy_assignable<const_array>::value );
	CHECK_FALSE( std::is_convertible<const array&, const_array>::value );
}
