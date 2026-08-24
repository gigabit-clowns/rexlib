// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/typed_program_cache_key.hpp>

#include <functional>
#include <string>

using namespace rexlib;

TEST_CASE(
	"typed_program_cache_key forwards to std::hash",
	"[typed_program_cache_key]"
)
{
	const typed_program_cache_key<int> key(42);
	CHECK(key.hash() == std::hash<int>{}(42));
}

TEST_CASE(
	"typed_program_cache_key exposes the wrapped value",
	"[typed_program_cache_key]"
)
{
	const typed_program_cache_key<std::string> key("plan-1024");
	CHECK(key.get_key() == "plan-1024");
}

TEST_CASE(
	"typed_program_cache_key equates same-typed keys by value",
	"[typed_program_cache_key]"
)
{
	const typed_program_cache_key<int> a(7);
	const typed_program_cache_key<int> b(7);
	const typed_program_cache_key<int> c(8);

	CHECK(a.equals(b));
	CHECK_FALSE(a.equals(c));
}
