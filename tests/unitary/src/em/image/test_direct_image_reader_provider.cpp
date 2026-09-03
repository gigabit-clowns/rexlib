// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/direct_image_reader_provider.hpp>

#include "mock/mock_image_reader.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A format that claims every file and counts how often it is asked to open
// one. The manager is final, so counting opens means counting them here.
class counting_format final
	: public image_read_format
{
public:
	explicit counting_format(std::shared_ptr<std::size_t> count)
		: m_count(std::move(count))
	{
	}

	std::string get_name() const override
	{
		return "counting";
	}

	backend_priority get_suitability(const image_probe &) const override
	{
		return backend_priority::normal;
	}

	std::unique_ptr<image_reader> open(const image_probe &) const override
	{
		++(*m_count);
		return std::unique_ptr<image_reader>(new mock_image_reader());
	}

private:
	std::shared_ptr<std::size_t> m_count;
};

std::shared_ptr<const image_read_format_manager> make_manager(
	const std::shared_ptr<std::size_t> &count
)
{
	auto manager = std::make_shared<image_read_format_manager>();
	manager->register_format(
		std::unique_ptr<image_read_format>(new counting_format(count))
	);
	return manager;
}

} // namespace

TEST_CASE( "a direct reader provider needs a format manager",
	"[direct_image_reader_provider]" )
{
	REQUIRE_THROWS_AS(
		direct_image_reader_provider(nullptr),
		std::invalid_argument
	);
}

TEST_CASE( "a direct reader provider opens a file every time it is asked",
	"[direct_image_reader_provider]" )
{
	const auto count = std::make_shared<std::size_t>(0);
	direct_image_reader_provider provider(make_manager(count));

	SECTION( "one request opens the file once" )
	{
		const auto reader = provider.acquire("stack_0.mrcs");

		REQUIRE( reader != nullptr );
		REQUIRE( *count == 1 );
	}

	SECTION( "the same path asked twice is opened twice" )
	{
		// It keeps nothing, which is what makes it the plain one.
		const auto first = provider.acquire("stack_0.mrcs");
		const auto second = provider.acquire("stack_0.mrcs");

		REQUIRE( *count == 2 );
		REQUIRE( first != second );
	}

	SECTION( "distinct paths are opened separately" )
	{
		provider.acquire("stack_0.mrcs");
		provider.acquire("stack_1.mrcs");

		REQUIRE( *count == 2 );
	}
}

TEST_CASE( "a direct reader provider reports what the manager reports",
	"[direct_image_reader_provider]" )
{
	// A manager with no format recognizes nothing, and the provider adds
	// no opinion of its own.
	direct_image_reader_provider provider(
		std::make_shared<image_read_format_manager>()
	);

	REQUIRE_THROWS_AS(
		provider.acquire("stack_0.mrcs"),
		invalid_operation_error
	);
}
