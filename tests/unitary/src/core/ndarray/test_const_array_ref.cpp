// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/ndarray/const_array_ref.hpp>

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/array_ref.hpp>
#include <rex/core/ndarray/const_array.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>

#include "../hardware/mock/mock_buffer.hpp"

#include <memory>
#include <type_traits>
#include <vector>

using namespace rexlib;

TEST_CASE("Default constructed const_array_ref should have no storage", "[const_array_ref]")
{
	const_array_ref ref;
	CHECK( ref.get_storage() == nullptr );
	CHECK( ref.share_storage() == nullptr );
	CHECK( ref.get_implementation() == nullptr );
}

TEST_CASE("Default constructed const_array_ref should have an empty descriptor", "[const_array_ref]")
{
	const_array_ref ref;
	CHECK( ref.get_descriptor() == array_descriptor() );
}

TEST_CASE("const_array_ref should reference the contents of the source", "[const_array_ref]")
{
	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();
	const std::vector<std::size_t> extents = {20, 50};
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32
	);

	array arr(storage, descriptor);

	const_array_ref ref;
	SECTION("from an array")
	{
		ref = arr;
	}
	SECTION("from a const_array")
	{
		const const_array alias = arr.share_const();
		ref = alias;
	}
	SECTION("from an array_ref")
	{
		const array_ref mutable_ref = arr;
		ref = mutable_ref;
	}

	CHECK( ref.get_implementation() == arr.get_implementation() );
	CHECK( ref.get_descriptor() == descriptor );
	CHECK( ref.get_storage() == arr.get_storage() );
	CHECK( ref.share_storage() == arr.share_storage() );
}

TEST_CASE("const_array_ref should be copyable", "[const_array_ref]")
{
	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();
	const array arr(storage, array_descriptor());

	const const_array_ref ref = arr;
	const const_array_ref copy = ref; // NOLINT
	CHECK( copy.get_implementation() == ref.get_implementation() );
}

TEST_CASE("const_array_ref should not extend ownership", "[const_array_ref]")
{
	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();
	const array arr(storage, array_descriptor());
	const auto use_count = storage.use_count();

	const const_array_ref ref = arr;
	CHECK( storage.use_count() == use_count );
	CHECK( ref.get_storage() == storage.get() );
}

TEST_CASE("const_array_ref should be constructible from the whole family", "[const_array_ref]")
{
	CHECK( std::is_convertible<const array&, const_array_ref>::value );
	CHECK( std::is_convertible<const const_array&, const_array_ref>::value );
	CHECK( std::is_convertible<array_ref, const_array_ref>::value );
	CHECK_FALSE( std::is_constructible<const_array, const_array_ref>::value );
}
