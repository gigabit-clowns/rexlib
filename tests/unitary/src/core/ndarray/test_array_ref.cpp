// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/ndarray/array_ref.hpp>

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>

#include "../hardware/mock/mock_buffer.hpp"

#include <memory>
#include <type_traits>
#include <vector>

using namespace rexlib;

TEST_CASE("Default constructed array_ref should have no storage", "[array_ref]")
{
	array_ref ref;
	CHECK( ref.get_storage() == nullptr );
	CHECK( ref.share_storage() == nullptr );
	CHECK( ref.get_implementation() == nullptr );
}

TEST_CASE("Default constructed array_ref should have an empty descriptor", "[array_ref]")
{
	array_ref ref;
	CHECK( ref.get_descriptor() == array_descriptor() );
}

TEST_CASE("array_ref should reference the contents of an array", "[array_ref]")
{
	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();
	const std::vector<std::size_t> extents = {20, 50};
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32
	);

	array arr(storage, descriptor);
	array_ref ref = arr;

	CHECK( ref.get_implementation() == arr.get_implementation() );
	CHECK( ref.get_descriptor() == descriptor );
	CHECK( ref.get_storage() == arr.get_storage() );
	CHECK( ref.share_storage() == arr.share_storage() );

	// The mutable overload is available on a non-const reference.
	buffer *mutable_storage = ref.get_storage();
	CHECK( mutable_storage == arr.get_storage() );
}

TEST_CASE("array_ref should be copyable", "[array_ref]")
{
	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();
	array arr(storage, array_descriptor());

	const array_ref ref = arr;
	const array_ref copy = ref; // NOLINT
	CHECK( copy.get_implementation() == ref.get_implementation() );
}

TEST_CASE("array_ref should not extend ownership", "[array_ref]")
{
	const std::shared_ptr<mock_buffer> storage =
		std::make_shared<mock_buffer>();
	array arr(storage, array_descriptor());
	const auto use_count = storage.use_count();

	const array_ref ref = arr;
	CHECK( storage.use_count() == use_count );
	CHECK( ref.get_storage() == storage.get() );
}

TEST_CASE("array_ref should only be constructible from a mutable array", "[array_ref]")
{
	CHECK( std::is_convertible<array&, array_ref>::value );
	CHECK_FALSE( std::is_convertible<const array&, array_ref>::value );
	CHECK_FALSE( std::is_constructible<array_ref, const_array&>::value );
	CHECK_FALSE( std::is_constructible<array_ref, const_array_ref>::value );
	CHECK_FALSE( std::is_constructible<array, array_ref>::value );
}
