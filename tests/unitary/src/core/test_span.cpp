// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/span.hpp>

#include <array>

using namespace rex;

TEST_CASE( "span default construction yields empty span", "[span]" )
{
	span<int, 0> empty_span;
	REQUIRE(empty_span.size() == 0);
	REQUIRE(empty_span.data() == nullptr);

	span<int> dynamic_span;
	REQUIRE(dynamic_span.size() == 0);
	REQUIRE(dynamic_span.data() == nullptr);
}

TEST_CASE( "span construction from data and size", "[span]" )
{
	std::array<int, 8> data;

	span<int, 8> static_span(data.data(), data.size());
	REQUIRE(static_span.size() == data.size());
	REQUIRE(static_span.data() == data.data());

	span<int> dynamic_span(data.data(), data.size());
	REQUIRE(dynamic_span.size() == data.size());
	REQUIRE(dynamic_span.data() == data.data());
}

TEST_CASE( "span copy construction preserves data and size", "[span]" )
{
	std::array<int, 8> data;

	span<int, 8> static_span(data.data(), data.size());
	auto static_span2 = static_span;
	REQUIRE(static_span2.size() == data.size());
	REQUIRE(static_span2.data() == data.data());

	span<int> dynamic_span(data.data(), data.size());
	auto dynamic_span2 = dynamic_span;
	REQUIRE(dynamic_span2.size() == data.size());
	REQUIRE(dynamic_span2.data() == data.data());
}
