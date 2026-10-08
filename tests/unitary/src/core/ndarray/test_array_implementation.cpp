// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <core/ndarray/array_implementation.hpp>

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include "../hardware/mock/mock_buffer.hpp"

#include <cstddef>
#include <memory>
#include <vector>

using namespace rexlib;

namespace
{

array_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	return array_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type
	);
}

} // anonymous namespace

TEST_CASE(
	"Default constructed array_implementation should hold nothing",
	"[array_implementation]"
)
{
	const array_implementation implementation;

	CHECK( implementation.get_storage() == nullptr );
	CHECK( implementation.share_storage() == nullptr );
	CHECK( implementation.get_access_hazard_tracker() == nullptr );
	CHECK( implementation.get_descriptor() == array_descriptor() );
}

TEST_CASE(
	"Constructing an array_implementation should store its attributes",
	"[array_implementation]"
)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor = make_descriptor({20, 50}, numerical_type::int32);

	const array_implementation implementation(storage, descriptor);

	CHECK( implementation.get_storage() == storage.get() );
	CHECK( implementation.share_storage() == storage );
	CHECK( implementation.get_descriptor() == descriptor );
}

TEST_CASE(
	"Constructing an array_implementation should give it a tracker of its "
	"own",
	"[array_implementation]"
)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor = make_descriptor({20, 50}, numerical_type::int32);

	const array_implementation first(storage, descriptor);
	const array_implementation second(storage, descriptor);

	REQUIRE( first.get_access_hazard_tracker() != nullptr );
	REQUIRE( second.get_access_hazard_tracker() != nullptr );
	CHECK(
		first.get_access_hazard_tracker() !=
		second.get_access_hazard_tracker()
	);
}

TEST_CASE(
	"Constructing an array_implementation from another should share its "
	"storage and its tracker under the new descriptor",
	"[array_implementation]"
)
{
	const auto storage = std::make_shared<mock_buffer>();
	const array_implementation source(
		storage,
		make_descriptor({20, 50}, numerical_type::int32)
	);
	const auto descriptor = make_descriptor({10, 100}, numerical_type::int32);

	const array_implementation view(source, descriptor);

	CHECK( view.share_storage() == storage );
	CHECK( view.get_descriptor() == descriptor );
	CHECK(
		view.get_access_hazard_tracker() ==
		source.get_access_hazard_tracker()
	);
}

TEST_CASE(
	"Copying an array_implementation should share its tracker",
	"[array_implementation]"
)
{
	const array_implementation source(
		std::make_shared<mock_buffer>(),
		make_descriptor({20, 50}, numerical_type::int32)
	);

	const array_implementation copy(source);

	CHECK( copy.share_storage() == source.share_storage() );
	CHECK( copy.get_descriptor() == source.get_descriptor() );
	CHECK(
		copy.get_access_hazard_tracker() ==
		source.get_access_hazard_tracker()
	);
}
