// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rex/core/ndarray/array_descriptor.hpp>

using namespace rex;

TEST_CASE("Default constructed array_descriptor should have an empty layout and unknown numerical type", "[array_descriptor]")
{
	array_descriptor descriptor;
	CHECK( descriptor.get_layout() == strided_layout() );
	CHECK( descriptor.get_data_type() == numerical_type::unknown );
}

TEST_CASE("Constructing an array_descriptor should store its attributes", "[array_descriptor]")
{
	const auto data_type = GENERATE(
		numerical_type::int16,
		numerical_type::float32
	);
	const auto extents = GENERATE(
		std::vector<std::size_t>{1, 2, 3},
		std::vector<std::size_t>{20, 30, 2, 16}
	);

	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));

	array_descriptor descriptor(layout, data_type);
	CHECK( descriptor.get_layout() == layout );
	CHECK( descriptor.get_data_type() == data_type );
}

TEST_CASE("make_contiguous_array_descriptor should wire its arguments into the descriptor", "[array_descriptor]")
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto data_type = numerical_type::float32;

	const auto descriptor = make_contiguous_array_descriptor(
		make_span(extents),
		data_type
	);

	CHECK( descriptor.get_layout() == strided_layout::make_contiguous_layout(make_span(extents)) );
	CHECK( descriptor.get_data_type() == data_type );
}

TEST_CASE("Computing the storage size for an array_descriptor should return the expected value", "[array_descriptor]")
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;

	array_descriptor descriptor(layout, data_type);
	CHECK( compute_storage_requirement(descriptor) == 20*30*2*16*4 );
}

TEST_CASE("Array descriptors should be equal if the data_type and layout are equal", "[array_descriptor]")
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;

	const array_descriptor descriptor1(layout, data_type);
	const array_descriptor descriptor2(layout, data_type);
	CHECK( descriptor1 == descriptor2 );
	CHECK( (descriptor1 != descriptor2) == false );
}

TEST_CASE("Array descriptors should be unequal if either data_type or layout are mismatch", "[array_descriptor]")
{
	const std::vector<std::size_t> extents1 = {20, 30, 2, 16};
	const std::vector<std::size_t> extents2 = {20, 30, 3, 16};
	const auto layout1 = strided_layout::make_contiguous_layout(make_span(extents1));
	const auto layout2 = strided_layout::make_contiguous_layout(make_span(extents2));
	const auto data_type1 = numerical_type::float32;
	const auto data_type2 = numerical_type::int16;

	CHECK( array_descriptor(layout1, data_type1) != array_descriptor(layout2, data_type1) );
	CHECK( array_descriptor(layout1, data_type1) != array_descriptor(layout1, data_type2) );
	CHECK( array_descriptor(layout1, data_type1) != array_descriptor(layout2, data_type2) );
}

TEST_CASE("value returned specialization of std::hash<array_descriptor> should be the one returned by array_descriptor::hash", "[array_descriptor]")
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const array_descriptor descriptor(layout, numerical_type::int32);
	std::hash<array_descriptor> hasher;
	CHECK(descriptor.hash() == hasher(descriptor) );
}

TEST_CASE("hash value of two equal array descriptors should be equal", "[array_descriptor]")
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const array_descriptor descriptor1(layout, numerical_type::int32);
	const array_descriptor descriptor2(layout, numerical_type::int32);
	CHECK( descriptor1.hash() == descriptor2.hash() );
}

TEST_CASE("hash value of array_descriptor should be different for different layouts", "[array_descriptor]")
{
	const std::vector<std::size_t> extents1 = {20, 30, 2, 16};
	const std::vector<std::size_t> extents2 = {20, 30, 1, 16};
	const auto layout1 = strided_layout::make_contiguous_layout(make_span(extents1));
	const auto layout2 = strided_layout::make_contiguous_layout(make_span(extents2));
	const array_descriptor descriptor1(layout1, numerical_type::int32);
	const array_descriptor descriptor2(layout2, numerical_type::int32);
	CHECK( descriptor1.hash() != descriptor2.hash() );
}

TEST_CASE("hash value of array_descriptor should be different for different data types", "[array_descriptor]")
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout = strided_layout::make_contiguous_layout(make_span(extents));
	const array_descriptor descriptor1(layout, numerical_type::int32);
	const array_descriptor descriptor2(layout, numerical_type::uint32);
	CHECK( descriptor1.hash() != descriptor2.hash() );
}
