// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/dispatch/program_cache.hpp>
#include <xmipp4/core/dispatch/typed_program_cache_key.hpp>

#include <memory>
#include <stdexcept>
#include <string>

using namespace rex;

namespace
{

using int_key = typed_program_cache_key<int>;
using long_key = typed_program_cache_key<long>;
using string_key = typed_program_cache_key<std::string>;

std::unique_ptr<int_key> make_int_key(int v)
{
	return std::make_unique<int_key>(v);
}

std::unique_ptr<long_key> make_long_key(long v)
{
	return std::make_unique<long_key>(v);
}

std::unique_ptr<string_key> make_string_key(std::string v)
{
	return std::make_unique<string_key>(std::move(v));
}

} // namespace

TEST_CASE(
	"program_cache reports the capacity it was built with",
	"[program_cache]"
)
{
	const program_cache cache(4);
	CHECK(cache.get_capacity() == 4);
}

TEST_CASE(
	"program_cache returns nullptr when touching a missing key",
	"[program_cache]"
)
{
	program_cache cache(4);
	const int_key probe(42);
	CHECK(cache.touch(probe) == nullptr);
}

TEST_CASE(
	"program_cache returns the stored value on a hit",
	"[program_cache]"
)
{
	program_cache cache(4);
	const auto value = std::make_shared<int>(123);

	cache.store(make_int_key(42), value);

	const int_key probe(42);
	CHECK(cache.touch(probe) == value);
}

TEST_CASE(
	"program_cache replaces the value when the same key is stored "
	"twice",
	"[program_cache]"
)
{
	program_cache cache(4);
	const auto first = std::make_shared<int>(1);
	const auto second = std::make_shared<int>(2);

	cache.store(make_int_key(42), first);
	cache.store(make_int_key(42), second);

	const int_key probe(42);
	CHECK(cache.touch(probe) == second);
}

TEST_CASE(
	"program_cache keeps keys of different types in disjoint name "
	"spaces",
	"[program_cache]"
)
{
	program_cache cache(4);
	const auto value_int = std::make_shared<int>(1);
	const auto value_long = std::make_shared<int>(2);

	// Same numeric value, different key types.
	cache.store(make_int_key(42), value_int);
	cache.store(make_long_key(42), value_long);

	const int_key int_probe(42);
	const long_key long_probe(42);
	CHECK(cache.touch(int_probe) == value_int);
	CHECK(cache.touch(long_probe) == value_long);
}

TEST_CASE(
	"program_cache evicts the oldest entry when storing past "
	"capacity",
	"[program_cache]"
)
{
	program_cache cache(2);
	const auto v1 = std::make_shared<int>(1);
	const auto v2 = std::make_shared<int>(2);
	const auto v3 = std::make_shared<int>(3);

	cache.store(make_int_key(1), v1);
	cache.store(make_int_key(2), v2);
	cache.store(make_int_key(3), v3);

	const int_key probe1(1);
	const int_key probe2(2);
	const int_key probe3(3);
	CHECK(cache.touch(probe1) == nullptr);
	CHECK(cache.touch(probe2) == v2);
	CHECK(cache.touch(probe3) == v3);
}

TEST_CASE(
	"program_cache promotes touched entries away from eviction",
	"[program_cache]"
)
{
	program_cache cache(2);
	const auto v1 = std::make_shared<int>(1);
	const auto v2 = std::make_shared<int>(2);
	const auto v3 = std::make_shared<int>(3);

	cache.store(make_int_key(1), v1);
	cache.store(make_int_key(2), v2);

	const int_key probe1(1);
	REQUIRE(cache.touch(probe1) == v1); // Touch promotes key 1.

	cache.store(make_int_key(3), v3);   // Should evict key 2, not key 1.

	const int_key probe2(2);
	const int_key probe3(3);
	CHECK(cache.touch(probe1) == v1);
	CHECK(cache.touch(probe2) == nullptr);
	CHECK(cache.touch(probe3) == v3);
}

TEST_CASE(
	"program_cache eviction crosses key-type partitions",
	"[program_cache]"
)
{
	program_cache cache(2);
	const auto v1 = std::make_shared<int>(1);
	const auto v2 = std::make_shared<int>(2);
	const auto v3 = std::make_shared<int>(3);

	cache.store(make_int_key(1), v1);
	cache.store(make_long_key(2), v2);
	cache.store(make_string_key("three"), v3);

	const int_key int_probe(1);
	const long_key long_probe(2);
	const string_key string_probe("three");
	CHECK(cache.touch(int_probe) == nullptr);
	CHECK(cache.touch(long_probe) == v2);
	CHECK(cache.touch(string_probe) == v3);
}

TEST_CASE(
	"program_cache::store throws when given a null key",
	"[program_cache]"
)
{
	program_cache cache(4);
	const auto value = std::make_shared<int>(1);

	REQUIRE_THROWS_AS(
		cache.store(nullptr, value),
		std::invalid_argument
	);
}

TEST_CASE(
	"program_cache::store throws when given a null value",
	"[program_cache]"
)
{
	program_cache cache(4);

	REQUIRE_THROWS_AS(
		cache.store(make_int_key(1), nullptr),
		std::invalid_argument
	);
}
