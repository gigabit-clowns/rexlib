// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/strided_pointer_iterator.hpp>

#include <array>
#include <algorithm>
#include <iterator>
#include <type_traits>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"strided_pointer_iterator exposes the expected iterator traits",
	"[strided_pointer_iterator]"
)
{
	using iterator = strided_pointer_iterator<int, std::ptrdiff_t>;
	STATIC_REQUIRE( std::is_same<iterator::value_type, int>::value );
	STATIC_REQUIRE( std::is_same<iterator::reference, int&>::value );
	STATIC_REQUIRE( std::is_same<iterator::pointer, int*>::value );
	STATIC_REQUIRE(
		std::is_same<iterator::difference_type, std::ptrdiff_t>::value
	);
	STATIC_REQUIRE( std::is_same<iterator::stride_type, std::ptrdiff_t>::value );
	STATIC_REQUIRE(
		std::is_same<
			iterator::iterator_category,
			std::bidirectional_iterator_tag
		>::value
	);

	// The value type drops cv-qualifiers while the reference keeps them.
	using const_iterator = strided_pointer_iterator<const int, std::ptrdiff_t>;
	STATIC_REQUIRE( std::is_same<const_iterator::value_type, int>::value );
	STATIC_REQUIRE(
		std::is_same<const_iterator::reference, const int&>::value
	);
}

TEST_CASE(
	"strided_pointer_iterator with a static stride keeps a signed integer "
	"difference type",
	"[strided_pointer_iterator]"
)
{
	using static_stride = std::integral_constant<std::ptrdiff_t, 4>;
	using iterator = strided_pointer_iterator<int, static_stride>;

	// The stride type is carried as-is, but the difference type stays a plain
	// signed integer so that indices and counts remain runtime values.
	STATIC_REQUIRE( std::is_same<iterator::stride_type, static_stride>::value );
	STATIC_REQUIRE(
		std::is_same<iterator::difference_type, std::ptrdiff_t>::value
	);
}

TEST_CASE(
	"make_strided_pointer_iterator stores the pointer and the stride",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 8> data;

	const auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});

	CHECK( it.data() == data.data() );
	CHECK( it.stride() == 2 );
}

TEST_CASE(
	"strided_pointer_iterator default constructor yields a null iterator",
	"[strided_pointer_iterator]"
)
{
	const strided_pointer_iterator<int, std::ptrdiff_t> it;

	CHECK( it.data() == nullptr );
	CHECK( it.stride() == 0 );
}

TEST_CASE(
	"strided_pointer_iterator dereferences the pointed element",
	"[strided_pointer_iterator]"
)
{
	const std::array<int, 4> data = { 10, 20, 30, 40 };

	const auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{1});

	CHECK( *it == 10 );
}

TEST_CASE(
	"strided_pointer_iterator provides member access through the arrow "
	"operator",
	"[strided_pointer_iterator]"
)
{
	struct point { int x; int y; };
	const std::array<point, 4> data = {{ {0, 0}, {1, 1}, {2, 2}, {3, 3} }};

	auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});

	CHECK( it->x == 0 );

	++it; // Advances two elements, landing on data[2].
	CHECK( it->x == 2 );
	CHECK( it->y == 2 );
}

TEST_CASE(
	"strided_pointer_iterator indexes relative to the stride",
	"[strided_pointer_iterator]"
)
{
	const std::array<int, 8> data = { 0, 1, 2, 3, 4, 5, 6, 7 };

	const auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});

	CHECK( it[0] == 0 );
	CHECK( it[1] == 2 );
	CHECK( it[3] == 6 );
}

TEST_CASE(
	"strided_pointer_iterator increment advances by the stride",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 8> data;

	auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});

	const auto previous = it++;
	CHECK( previous.data() == data.data() );
	CHECK( it.data() == data.data() + 2 );

	++it;
	CHECK( it.data() == data.data() + 4 );
}

TEST_CASE(
	"strided_pointer_iterator decrement steps back by the stride",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 8> data;

	auto it = make_strided_pointer_iterator(data.data() + 4, std::ptrdiff_t{2});

	const auto previous = it--;
	CHECK( previous.data() == data.data() + 4 );
	CHECK( it.data() == data.data() + 2 );

	--it;
	CHECK( it.data() == data.data() );
}

TEST_CASE(
	"strided_pointer_iterator compound assignment moves by multiples of the "
	"stride",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 16> data;

	auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{3});

	it += 4;
	CHECK( it.data() == data.data() + 12 );

	it -= 2;
	CHECK( it.data() == data.data() + 6 );
}

TEST_CASE(
	"strided_pointer_iterator arithmetic returns advanced iterators without "
	"modifying the operand",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 16> data;

	const auto it = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{3});

	CHECK( (it + 2).data() == data.data() + 6 );
	CHECK( (2 + it).data() == data.data() + 6 );
	CHECK( (it + 4 - 1).data() == data.data() + 9 );

	// The original iterator is left untouched.
	CHECK( it.data() == data.data() );
}

TEST_CASE(
	"strided_pointer_iterator compares by the pointed element",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 8> data;

	const auto first = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});
	const auto same = make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});
	const auto other =
		make_strided_pointer_iterator(data.data() + 2, std::ptrdiff_t{2});

	CHECK( first == same );
	CHECK( first != other );
	CHECK_FALSE( first == other );

	// Only the pointer takes part in the comparison; the stride is ignored.
	const auto different_stride =
		make_strided_pointer_iterator(data.data(), std::ptrdiff_t{5});
	CHECK( first == different_stride );
}

TEST_CASE(
	"strided_pointer_iterator implicitly converts from mutable to constant",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 4> data = { 10, 20, 30, 40 };

	const auto mutable_it =
		make_strided_pointer_iterator(data.data(), std::ptrdiff_t{2});
	const strided_pointer_iterator<const int, std::ptrdiff_t> const_it =
		mutable_it;

	CHECK( const_it.data() == data.data() );
	CHECK( const_it.stride() == 2 );
	CHECK( *const_it == 10 );
}

TEST_CASE(
	"strided_pointer_iterator reads a strided range with a standard algorithm",
	"[strided_pointer_iterator]"
)
{
	const std::array<int, 8> source = { 0, 1, 2, 3, 4, 5, 6, 7 };
	std::array<int, 4> result;

	const auto first = make_strided_pointer_iterator(source.data(), std::ptrdiff_t{2});
	std::copy_n(first, result.size(), result.begin());

	const std::array<int, 4> expected = { 0, 2, 4, 6 };
	CHECK( result == expected );
}

TEST_CASE(
	"strided_pointer_iterator writes a strided range with a standard algorithm",
	"[strided_pointer_iterator]"
)
{
	std::array<int, 8> buffer = { 0, 0, 0, 0, 0, 0, 0, 0 };

	const auto first = make_strided_pointer_iterator(buffer.data(), std::ptrdiff_t{2});
	std::fill_n(first, 4, 7);

	const std::array<int, 8> expected = { 7, 0, 7, 0, 7, 0, 7, 0 };
	CHECK( buffer == expected );
}

TEST_CASE(
	"strided_pointer_iterator advances by a compile-time stride using runtime "
	"indices",
	"[strided_pointer_iterator]"
)
{
	using static_stride = std::integral_constant<std::ptrdiff_t, 2>;
	const std::array<int, 8> data = { 0, 1, 2, 3, 4, 5, 6, 7 };

	auto it = make_strided_pointer_iterator(data.data(), static_stride{});

	// The stride keeps its compile-time type but indices stay runtime values.
	STATIC_REQUIRE(
		std::is_same<decltype(it.stride()), static_stride>::value
	);
	CHECK( it.stride() == 2 );

	CHECK( *it == 0 );
	CHECK( it[3] == 6 );

	++it;
	CHECK( *it == 2 );

	it += 2;
	CHECK( *it == 6 );
}

TEST_CASE(
	"strided_pointer_iterator reads a compile-time strided range with a "
	"standard algorithm",
	"[strided_pointer_iterator]"
)
{
	using static_stride = std::integral_constant<std::ptrdiff_t, 2>;
	const std::array<int, 8> source = { 0, 1, 2, 3, 4, 5, 6, 7 };
	std::array<int, 4> result;

	const auto first = make_strided_pointer_iterator(source.data(), static_stride{});
	std::copy_n(first, result.size(), result.begin());

	const std::array<int, 4> expected = { 0, 2, 4, 6 };
	CHECK( result == expected );
}

TEST_CASE(
	"strided_pointer_iterator converts to constant preserving the stride type",
	"[strided_pointer_iterator]"
)
{
	using static_stride = std::integral_constant<std::ptrdiff_t, 2>;
	using const_iterator = strided_pointer_iterator<const int, static_stride>;
	std::array<int, 4> data = { 10, 20, 30, 40 };

	const auto mutable_it =
		make_strided_pointer_iterator(data.data(), static_stride{});
	const const_iterator const_it = mutable_it;

	CHECK( const_it.data() == data.data() );
	CHECK( const_it.stride() == 2 );
	CHECK( *const_it == 10 );
}
