// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read_format_manager.hpp>

#include "fake/fake_image_reader.hpp"
#include "mock/mock_image_read_format.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/em/image/image_format_registry.hpp>
#include <rexlib/em/image/image_open_options.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::unique_ptr<image_reader> make_fake_reader()
{
	const std::vector<std::size_t> extents = {2, 2};
	return std::unique_ptr<image_reader>(new fake_image_reader(
		make_span(extents),
		image_metadata()
	));
}

class staged_format final
	: public image_read_format
{
public:
	staged_format(std::string name, backend_priority suitability)
		: m_name(std::move(name))
		, m_suitability(suitability)
	{
	}

	std::string get_name() const override
	{
		return m_name;
	}

	backend_priority get_suitability(const image_probe &) const override
	{
		return m_suitability;
	}

	std::unique_ptr<image_reader> open(
		const image_probe &,
		const image_open_options &
	) const override
	{
		return make_fake_reader();
	}

private:
	std::string m_name;
	backend_priority m_suitability;
};

std::unique_ptr<image_read_format> make_staged(
	std::string name,
	backend_priority suitability
)
{
	return std::unique_ptr<image_read_format>(
		new staged_format(std::move(name), suitability)
	);
}

} // namespace

TEST_CASE( "an empty read manager recognizes nothing",
	"[image_read_format_manager]" )
{
	const image_read_format_manager manager;

	SECTION( "no format claims a file" )
	{
		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}

	SECTION( "opening reports that nothing is suitable" )
	{
		REQUIRE_THROWS_AS(
			manager.open("absent.mrc", image_open_options()),
			invalid_operation_error
		);
	}
}

TEST_CASE( "the read manager picks the most suitable format",
	"[image_read_format_manager]" )
{
	image_read_format_manager manager;

	SECTION( "the only supporting format is chosen" )
	{
		manager.register_format(make_staged("only", backend_priority::normal));

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "only" );
	}

	SECTION( "the highest priority wins" )
	{
		manager.register_format(
			make_staged("fallback", backend_priority::fallback));
		manager.register_format(
			make_staged("optimal", backend_priority::optimal));
		manager.register_format(
			make_staged("normal", backend_priority::normal));

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "optimal" );
	}

	SECTION( "a format reporting unsupported is never chosen" )
	{
		manager.register_format(
			make_staged("declines", backend_priority::unsupported));
		manager.register_format(
			make_staged("accepts", backend_priority::fallback));

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "accepts" );
	}

	SECTION( "every format declining leaves nothing suitable" )
	{
		manager.register_format(
			make_staged("a", backend_priority::unsupported));
		manager.register_format(
			make_staged("b", backend_priority::unsupported));

		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
		REQUIRE_THROWS_AS(
			manager.open("absent.mrc", image_open_options()),
			invalid_operation_error
		);
	}

	SECTION( "the chosen format opens the reader" )
	{
		manager.register_format(make_staged("only", backend_priority::normal));

		const auto reader = manager.open(
			"absent.mrc",
			image_open_options()
		);

		REQUIRE( reader != nullptr );
		REQUIRE( reader->get_descriptor().get_data_type() ==
			numerical_type::int16 );
	}
}

TEST_CASE( "the read manager refuses a null format",
	"[image_read_format_manager]" )
{
	image_read_format_manager manager;

	REQUIRE_FALSE( manager.register_format(nullptr) );
	REQUIRE( manager.register_format(
		make_staged("real", backend_priority::normal)) );
}

TEST_CASE( "the read manager consults every registered format",
	"[image_read_format_manager]" )
{
	image_read_format_manager manager;

	auto first = std::unique_ptr<mock_image_read_format>(
		new mock_image_read_format());
	auto second = std::unique_ptr<mock_image_read_format>(
		new mock_image_read_format());

	REQUIRE_CALL(*first, get_suitability(ANY(const image_probe&)))
		.RETURN(backend_priority::unsupported);
	REQUIRE_CALL(*second, get_suitability(ANY(const image_probe&)))
		.RETURN(backend_priority::normal);
	ALLOW_CALL(*second, get_name()).RETURN(std::string("second"));

	const auto *expected = second.get();
	manager.register_format(std::move(first));
	manager.register_format(std::move(second));

	const auto *chosen = manager.get_most_suitable_format(
		image_probe("absent.mrc"));

	REQUIRE( chosen == expected );
}

TEST_CASE( "a read registry hands its formats to a manager",
	"[image_read_format_manager]" )
{
	image_read_format_registry registry;
	image_read_format_manager manager;

	SECTION( "a drained registry populates the manager" )
	{
		registry.add([] () -> std::unique_ptr<image_read_format>
		{
			return make_staged("registered", backend_priority::normal);
		});
		registry.register_all(manager);

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "registered" );
	}

	SECTION( "a null factory is ignored" )
	{
		registry.add(nullptr);
		registry.register_all(manager);

		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}
}
