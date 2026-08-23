// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <rex/core/layout/strided_layout.hpp>

#include <algorithm>
#include <array>

using namespace rex;

template <std::size_t N>
static 
strided_layout make_custom_layout(
	const std::array<std::size_t, N> &extents,
	const std::array<std::ptrdiff_t, N> &strides,
	std::ptrdiff_t offset
)
{
	return strided_layout::make_custom_layout(
		xmipp4::make_span(extents),
		xmipp4::make_span(strides),
		offset
	);
}

static 
strided_layout make_test_layout()
{
	const std::array<std::size_t, 6> extents = 
	{
		120, 
		56, 
		24, 
		1, 
		10, 
		8
	};
	const std::array<std::ptrdiff_t, 6> strides = 
	{       
		860160,
		7680,
		320,
		160,
		16,
		2
	};
	const auto offset = 20;

	return make_custom_layout(extents, strides, offset);
}

TEST_CASE("make_contiguous_layout should create a strided_layout with the expected extents and strides", "[strided_layout]")
{
	const std::vector<std::size_t> extents = {120, 56, 24, 1, 10, 8};
	const auto layout = strided_layout::make_contiguous_layout(
		xmipp4::make_span(extents)
	);

	std::vector<std::size_t> obtained_extents;
	layout.get_extents(obtained_extents);
	REQUIRE( obtained_extents == extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	layout.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		107520,
		1920,
		80,
		80,
		8,
		1
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( layout.get_offset() == 0 );
	REQUIRE( layout.get_rank() == extents.size() );
}

TEST_CASE("make_custom_layout should create a strided_layout with expected extents and strides", "[strided_layout]")
{
	const std::vector<std::size_t> extents = {120, 56, 24, 1, 10, 8};
	const std::vector<std::ptrdiff_t> strides = 
	{
		107520,
		7680,
		320,
		160,
		16,
		2
	};
	const auto offset = 1234;
	const auto layout = strided_layout::make_custom_layout(
		xmipp4::make_span(extents),
		xmipp4::make_span(strides),
		offset
	);

	std::vector<std::size_t> obtained_extents;
	layout.get_extents(obtained_extents);
	REQUIRE( obtained_extents == extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	layout.get_strides(obtained_strides);
	REQUIRE( obtained_strides == strides );

	REQUIRE( layout.get_offset() == offset );
	REQUIRE( layout.get_rank() == extents.size() );
}

TEST_CASE("make_custom_layout with inhomogeneous arrays should throw", "[strided_layout]")
{
	const std::vector<std::size_t> extents = {120, 56, 24, 1, 10, 8};
	const std::vector<std::ptrdiff_t> strides = 
	{
		107520,
		7680,
		160,
		16,
		2
	};
	const auto offset = 1234;
		
	REQUIRE_THROWS_MATCHES(
		strided_layout::make_custom_layout(
			xmipp4::make_span(extents),
			xmipp4::make_span(strides),
			offset
		),
		std::invalid_argument,
		Catch::Matchers::Message("Extents and strides must have the same number of elements")
	);
}

TEST_CASE("default constructed strided_layout should have no axes and an offset of zero", "[strided_layout]")
{
	const strided_layout layout;

	std::vector<std::size_t> obtained_extents;
	layout.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents;
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	layout.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides;
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( layout.get_offset() == 0 );
	REQUIRE( layout.get_rank() == 0 );
}

TEST_CASE("compute_storage_requirement in strided_layout should return the correct size", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::size_t expected_storage = 20 + 7*2 + 9*16 + 23*320 + 55*7680 + 119*860160 + 1;
	REQUIRE( layout.compute_storage_requirement() == expected_storage );
}

TEST_CASE("compute_storage_requirement in strided_layout should return 0 if there is a zero-sized axis", "[strided_layout]")
{
	std::array<std::size_t, 6> extents = 
	{
		120, 
		56, 
		0, 
		1, 
		10, 
		8
	};
	const auto layout = strided_layout::make_contiguous_layout(
		xmipp4::make_span(extents)
	);

	REQUIRE( layout.compute_storage_requirement() == 0 );
}

TEST_CASE("compute_element_count in strided_layout should return the hyper volume of the layout", "[strided_layout]")
{
	std::array<std::size_t, 6> extents = 
	{
		120, 
		56, 
		40, 
		1, 
		10, 
		8
	};
	const auto layout = strided_layout::make_contiguous_layout(
		xmipp4::make_span(extents)
	);

	REQUIRE( layout.compute_element_count() == 120*56*40*1*10*8 );
}

TEST_CASE("Calling extents_equal on a default constructed strided_layout should only return true if the provided extents are empty", "[strided_layout]")
{
	const std::array<std::size_t, 2> extents = {20, 4};

	strided_layout layout;

	CHECK( layout.extents_equal({}) == true );
	CHECK( layout.extents_equal(xmipp4::make_span(extents)) == false );
}

TEST_CASE("Calling extents_equal on a strided_layout should return false if different rank", "[strided_layout]")
{
	const std::array<std::size_t, 2> extents1 = {20, 4};
	const std::array<std::size_t, 1> extents2 = {4};

	const auto layout = strided_layout::make_contiguous_layout(xmipp4::make_span(extents1));

	CHECK( layout.extents_equal(xmipp4::make_span(extents2)) == false );
}

TEST_CASE("Calling extents_equal on a strided_layout should return false if the extent of an axis mismatches", "[strided_layout]")
{
	const std::array<std::size_t, 2> extents1 = {20, 4};
	const std::array<std::size_t, 2> extents2 = {20, 1};

	const auto layout = strided_layout::make_contiguous_layout(xmipp4::make_span(extents1));

	CHECK( layout.extents_equal(xmipp4::make_span(extents2)) == false );
}

TEST_CASE("Calling extents_equal on a strided_layout should return true if extents match", "[strided_layout]")
{
	const std::array<std::size_t, 4> extents1 = {20, 4, 60, 2};
	const std::array<std::size_t, 4> extents2 = {20, 4, 60, 2};

	const auto layout = strided_layout::make_contiguous_layout(xmipp4::make_span(extents1));

	CHECK( layout.extents_equal(xmipp4::make_span(extents2)) == true );
}

TEST_CASE( "apply_subscripts in strided_layout should implicitly fill the reminding axes", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		1
	};

	const auto result = layout.apply_subscripts(xmipp4::make_span(subscripts));

	std::vector<std::size_t> obtained_extents;
	result.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		56,
		24,
		1,
		10,
		8
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	result.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		7680,
		320,
		160,
		16,
		2
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( result.get_offset() == layout.get_offset() + 1*860160 );
}

TEST_CASE( "apply_subscripts in strided_layout should implicitly fill the reminding axes after an ellipsis", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		ellipsis(),
		1
	};

	const auto result = layout.apply_subscripts(xmipp4::make_span(subscripts));

	std::vector<std::size_t> obtained_extents;
	result.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		120,
		56,
		24,
		1,
		10
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	result.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		860160,
		7680,
		320,
		160,
		16
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( result.get_offset() == layout.get_offset() + 1*2 );
}

TEST_CASE( "apply_subscripts in strided_layout with a complex subscript should produce the expected result", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		odd(), // Selects half of the elements in the axis with 120 elements
		new_axis(), // Inserts phantom axis between 120 and 56 sized axes
		ellipsis(), // Absorbs 56 and 24 sized axes.
		0, // Squeezes 1-sized axis
		even(), // Selects half of the elements in the axis with 10 elements
		new_axis(), // Inserts an axis between 10 and 8 sized axes
		new_axis(), // Inserts an axis between 10 and 8 sized axes
		6 // Selects the 7th element in the 8 element axis
	};

	const auto result = layout.apply_subscripts(xmipp4::make_span(subscripts));

	std::vector<std::size_t> obtained_extents;
	result.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		60,
		1,
		56,
		24,
		5,
		1,
		1
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	result.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		1720320,
		0,
		7680,
		320,
		32,
		0,
		0
	};
	REQUIRE( obtained_strides == expected_strides );

	const std::ptrdiff_t expected_offset = 860192;
	REQUIRE( result.get_offset() == expected_offset );
}

TEST_CASE( "apply_subscripts with no subscripts in a default constructed layout should succeed", "[strided_layout]" )
{
	const strided_layout layout;
	const std::vector<dynamic_subscript> subscripts;

	const auto result = layout.apply_subscripts(xmipp4::make_span(subscripts));

	const strided_layout expected_layout;
	REQUIRE( result == expected_layout );
}

TEST_CASE( "apply_subscripts with an ellipsis in a default constructed layout should succeed", "[strided_layout]" )
{
	const strided_layout layout;
	const std::vector<dynamic_subscript> subscripts = 
	{
		ellipsis()
	};

	const auto result = layout.apply_subscripts(xmipp4::make_span(subscripts));

	const strided_layout expected_layout;
	REQUIRE( result == expected_layout );
}

TEST_CASE( "apply_subscripts with a new_axis in a default constructed layout should succeed", "[strided_layout]" )
{
	const strided_layout layout;
	const std::vector<dynamic_subscript> subscripts = 
	{
		new_axis()
	};

	const auto result = layout.apply_subscripts(xmipp4::make_span(subscripts));
	
	const std::array<std::size_t, 1> expected_extents = {1};
	const std::array<std::ptrdiff_t, 1> expected_strides = {0};
	const std::ptrdiff_t expected_offset = 0;
	const auto expected_layout = make_custom_layout(
		expected_extents,
		expected_strides,
		expected_offset
	);

	REQUIRE( result == expected_layout );
}

TEST_CASE( "apply_subscripts in strided_layout with two ellipsis should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		ellipsis(),
		new_axis(),
		ellipsis()
	};

	REQUIRE_THROWS_MATCHES(
		layout.apply_subscripts(xmipp4::make_span(subscripts)),
		std::invalid_argument,
		Catch::Matchers::Message("Two ellipsis tags were encountered when processing subscripts")
	);
}

TEST_CASE( "apply_subscripts in strided_layout with too many subscripts should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		6,
		2,
		2,
		odd(),
		even(),
		all(),
		2
	};

	REQUIRE_THROWS_MATCHES(
		layout.apply_subscripts(xmipp4::make_span(subscripts)),
		std::invalid_argument,
		Catch::Matchers::Message("An index subscript was encountered, but there are no more axes to process")
	);
}

TEST_CASE( "apply_subscripts in strided_layout with too many subscripts and ellipsis should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
			6,
			2,
			2,
			ellipsis(),
			odd(),
			even(),
			all(),
			2
	};

	REQUIRE_THROWS_MATCHES(
		layout.apply_subscripts(xmipp4::make_span(subscripts)),
		std::invalid_argument,
		Catch::Matchers::Message("A slice subscript was encountered, but there are no more axes to process")
	);
}

TEST_CASE( "apply_subscripts in strided_layout with out of bounds index should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		120
	};

	REQUIRE_THROWS_AS( layout.apply_subscripts(xmipp4::make_span(subscripts)), std::out_of_range );
}

TEST_CASE( "apply_subscripts in strided_layout with out of bounds index after ellipsis should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		ellipsis(),
		8
	};

	REQUIRE_THROWS_AS( layout.apply_subscripts(xmipp4::make_span(subscripts)), std::out_of_range );
}

TEST_CASE( "apply_subscripts in strided_layout with out of bounds slice should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		make_slice(121)
	};

	REQUIRE_THROWS_AS( layout.apply_subscripts(xmipp4::make_span(subscripts)), std::out_of_range );
}

TEST_CASE( "apply_subscripts in strided_layout with out of bounds slice after ellipsis should throw", "[strided_layout]" )
{
	const auto layout = make_test_layout();
	const std::vector<dynamic_subscript> subscripts = 
	{
		ellipsis(),
		make_slice(9)
	};

	REQUIRE_THROWS_AS( layout.apply_subscripts(xmipp4::make_span(subscripts)), std::out_of_range );
}

TEST_CASE( "apply_subscripts with an index in a default constructed layout should throw", "[strided_layout]" )
{
	const strided_layout layout;
	const std::vector<dynamic_subscript> subscripts = 
	{
		0
	};

	REQUIRE_THROWS_MATCHES(
		layout.apply_subscripts(xmipp4::make_span(subscripts)),
		std::invalid_argument,
		Catch::Matchers::Message("An index subscript was encountered, but there are no more axes to process")
	);
}

TEST_CASE( "apply_subscripts with an slice in a default constructed layout should throw", "[strided_layout]" )
{
	const strided_layout layout;
	const std::vector<dynamic_subscript> subscripts = 
	{
		make_slice(1)
	};

	REQUIRE_THROWS_MATCHES(
		layout.apply_subscripts(xmipp4::make_span(subscripts)),
		std::invalid_argument,
		Catch::Matchers::Message("A slice subscript was encountered, but there are no more axes to process")
	);
}

TEST_CASE("transpose in strided_layout should reverse the order of its axes", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const auto transposed = layout.transpose();

	std::vector<std::size_t> obtained_extents;
	transposed.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = {8, 10, 1, 24, 56, 120};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	transposed.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = {2, 16, 160, 320, 7680, 860160};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( transposed.get_offset() == layout.get_offset() );
}

TEST_CASE("transpose in a default constructed strided_layout should return another empty layout ", "[strided_layout]")
{
	const strided_layout layout;

	const auto transposed = layout.transpose();

	const strided_layout expected_layout;
	REQUIRE( transposed == expected_layout );
}

TEST_CASE("permute in strided_layout with valid permutation should correctly alter the order of the axes", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::vector<std::size_t> permutation = {0, 3, 1, 2, 4, 5};
	const auto permuted = layout.permute(xmipp4::make_span(permutation));

	std::vector<std::size_t> obtained_extents;
	permuted.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		120,
		1,
		56,
		24,
		10,
		8
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	permuted.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		860160,
		160,
		7680,
		320,
		16,
		2
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( permuted.get_offset() == layout.get_offset() );
}

TEST_CASE("permute in strided_layout with valid permutation should throw if not provided with a correct permutation", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::vector<std::size_t> permutation = {0, 1, 2, 3, 4, 4};

	REQUIRE_THROWS_MATCHES(
		layout.permute(xmipp4::make_span(permutation)),
		std::invalid_argument,
		Catch::Matchers::Message("Index 5 is missing in the axis permutation")
	);
}

TEST_CASE("permute in a default constructed strided_layout and empty permutation should succeed and return empty permutation", "[strided_layout]")
{
	const strided_layout layout;
	const std::vector<std::size_t> permutation;

	const auto permuted = layout.permute(xmipp4::make_span(permutation));

	const strided_layout expected_layout;
	REQUIRE( permuted == expected_layout );
}

TEST_CASE("permute in a default constructed strided_layout and non-empty permutation should throw", "[strided_layout]")
{
	const strided_layout layout;
	const std::vector<std::size_t> permutation = {0};

	REQUIRE_THROWS_MATCHES(
		layout.permute(xmipp4::make_span(permutation)),
		std::invalid_argument,
		Catch::Matchers::Message("Axis permutation's length does not match the required count")
	);
}

TEST_CASE("matrix_transpose in strided_layout should swap the requested axes", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const auto swapped = layout.matrix_transpose(1, 2);

	std::vector<std::size_t> obtained_extents;
	swapped.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		120,
		24,
		56,
		1,
		10,
		8
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	swapped.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		860160,
		320,
		7680,
		160,
		16,
		2
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( swapped.get_offset() == layout.get_offset() );
}

TEST_CASE("matrix_transpose in strided_layout should throw when one of the axes is out of bounds", "[strided_layout]")
{
	const auto layout = make_test_layout();

	REQUIRE_THROWS_MATCHES(
		layout.matrix_transpose(6, 0),
		std::out_of_range,
		Catch::Matchers::Message("Index 6 is out of bounds for extent 6")
	);
	REQUIRE_THROWS_MATCHES(
		layout.matrix_transpose(0, 6),
		std::out_of_range,
		Catch::Matchers::Message("Index 6 is out of bounds for extent 6")
	);
}

TEST_CASE("matrix_transpose in default constructed strided_layout should always fail", "[strided_layout]")
{
	const strided_layout layout;

	REQUIRE_THROWS_MATCHES(
		layout.matrix_transpose(0, 0),
		std::out_of_range,
		Catch::Matchers::Message("Index 0 is out of bounds for extent 0")
	);
	REQUIRE_THROWS_MATCHES(
		layout.matrix_transpose(1, 1),
		std::out_of_range,
		Catch::Matchers::Message("Index 1 is out of bounds for extent 0")
	);
}

TEST_CASE("matrix_diagonal in strided_layout should return a layout with the diagonal elements", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const auto swapped = layout.matrix_diagonal(-2, 1);

	std::vector<std::size_t> obtained_extents;
	swapped.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		120,
		24,
		1,
		8,
		10
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	swapped.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		860160,
		320,
		160,
		2,
		7680 + 16,
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( swapped.get_offset() == layout.get_offset() );
}

TEST_CASE("matrix_diagonal in strided_layout should throw when one of the axes is out of bounds", "[strided_layout]")
{
	const auto layout = make_test_layout();

	REQUIRE_THROWS_MATCHES(
		layout.matrix_diagonal(6, 0),
		std::out_of_range,
		Catch::Matchers::Message("Index 6 is out of bounds for extent 6")
	);
	REQUIRE_THROWS_MATCHES(
		layout.matrix_diagonal(0, 6),
		std::out_of_range,
		Catch::Matchers::Message("Index 6 is out of bounds for extent 6")
	);
}

TEST_CASE("matrix_diagonal in default constructed strided_layout should always fail", "[strided_layout]")
{
	const strided_layout layout;

	REQUIRE_THROWS_MATCHES(
		layout.matrix_diagonal(0, 0),
		std::out_of_range,
		Catch::Matchers::Message("Index 0 is out of bounds for extent 0")
	);
	REQUIRE_THROWS_MATCHES(
		layout.matrix_diagonal(1, 0),
		std::out_of_range,
		Catch::Matchers::Message("Index 1 is out of bounds for extent 0")
	);
}

TEST_CASE("squeeze in strided_layout should remove all axes of extent 1", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const auto squeezed = layout.squeeze();

	std::vector<std::size_t> obtained_extents;
	squeezed.get_extents(obtained_extents);
	const std::vector<std::size_t> expected_extents = 
	{
		120,
		56,
		24,
		10,
		8
	};
	REQUIRE( obtained_extents == expected_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	squeezed.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		860160,
		7680,
		320,
		16,
		2
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( squeezed.get_offset() == 20 );
}

TEST_CASE("squeeze in default constructed strided_layout should return an empty layout", "[strided_layout]")
{
	const strided_layout layout;
	const auto squeezed = layout.squeeze();

	const strided_layout expected_layout;
	REQUIRE( squeezed == expected_layout );
}

TEST_CASE("broadcast_to in strided_layout should fill in the left and promote axes with extent 1", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::vector<std::size_t> target_extents = 
	{
		16,
		40,
		120,
		56,
		24,
		9,
		10,
		8
	};

	const auto broadcasted = layout.broadcast_to(xmipp4::make_span(target_extents));

	std::vector<std::size_t> obtained_extents;
	broadcasted.get_extents(obtained_extents);
	REQUIRE( obtained_extents == target_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	broadcasted.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides = 
	{
		0,
		0,
		860160,
		7680,
		320,
		0,
		16,
		2
	};
	REQUIRE( obtained_strides == expected_strides );

	REQUIRE( broadcasted.get_offset() == 20 );
}

TEST_CASE("broadcast_to in strided_layout should alias when extents are unchanged", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::vector<std::size_t> target_extents = 
	{
		120, 
		56, 
		24, 
		1, 
		10, 
		8
	};

	const auto broadcasted = layout.broadcast_to(xmipp4::make_span(target_extents));


	CHECK( &broadcasted.get_implementation() == &layout.get_implementation() );
}

TEST_CASE("broadcast_to in a default constructed strided_layout should succeed", "[strided_layout]")
{
	strided_layout layout;
	const std::vector<std::size_t> target_extents = 
	{
		120, 
		56, 
		24, 
		1, 
		10, 
		8
	};

	const auto broadcasted = layout.broadcast_to(xmipp4::make_span(target_extents));

	std::vector<std::size_t> obtained_extents;
	broadcasted.get_extents(obtained_extents);
	CHECK( obtained_extents == target_extents );

	std::vector<std::ptrdiff_t> obtained_strides;
	broadcasted.get_strides(obtained_strides);
	const std::vector<std::ptrdiff_t> expected_strides(target_extents.size(), 0);
	CHECK( obtained_strides == expected_strides );

	CHECK( broadcasted.get_offset() == 0 );
}

TEST_CASE("broadcast_to in strided_layout should throw when the provided extents have less axes than the layout", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::vector<std::size_t> target_extents = {56, 24, 1, 10, 8};

	REQUIRE_THROWS_MATCHES(
		layout.broadcast_to(xmipp4::make_span(target_extents)),
		std::invalid_argument,
		Catch::Matchers::Message("Cannot broadcast layout with 6 axes into a shape of 5 dimensions.")
	);
}

TEST_CASE("broadcast_to in strided_layout should throw if an axis is not broadcastable", "[strided_layout]")
{
	const auto layout = make_test_layout();
	const std::vector<std::size_t> target_extents = {120, 55, 24, 1, 10, 8};

	REQUIRE_THROWS_MATCHES(
		layout.broadcast_to(xmipp4::make_span(target_extents)),
		std::invalid_argument,
		Catch::Matchers::Message("Cannot broadcast axis of extent 56 into an extent of 55.")
	);
}

TEST_CASE("value returned specialization of std::hash<strided_layout> should be the one returned by strided_layout::hash", "[strided_layout]")
{
	const auto layout = make_test_layout();
	std::hash<strided_layout> hasher;
	CHECK(layout.hash() == hasher(layout) );
}

TEST_CASE("hash of a default-constructed strided_layout should be zero", "[strided_layout]")
{
	const strided_layout layout;
	CHECK( layout.hash() == 0 );
}

TEST_CASE("hash of a strided_layout should change with axis ordering", "[strided_layout]")
{
	const auto layout = make_test_layout();
	CHECK(layout.hash() != layout.matrix_transpose().hash());
}

TEST_CASE("hash of two equal strided_layout should be equal", "[strided_layout]")
{
	const auto layout1 = make_test_layout();
	const auto layout2 = make_test_layout();
	CHECK(layout1.hash() == layout2.hash());
}

TEST_CASE("hash of a strided_layout should change when adding a phantom axis", "[strided_layout]")
{	
	const auto layout = make_test_layout();
	const std::array<dynamic_subscript, 1> subscripts = { new_axis() };
	CHECK(layout.hash() != layout.apply_subscripts(xmipp4::make_span(subscripts)).hash());
}

TEST_CASE("hash of a strided_layout should change when changing the offset", "[strided_layout]")
{	

	const std::vector<std::size_t> extents = {8, 10, 1, 24, 56, 120};
	const std::vector<std::ptrdiff_t> strides = {2, 16, 160, 320, 7680, 860160};
	const auto layout1 = strided_layout::make_custom_layout(
		xmipp4::make_span(extents),
		xmipp4::make_span(strides),
		1
	);
	const auto layout2 = strided_layout::make_custom_layout(
		xmipp4::make_span(extents),
		xmipp4::make_span(strides),
		2
	);
	CHECK(layout1.hash() != layout2.hash());
}
