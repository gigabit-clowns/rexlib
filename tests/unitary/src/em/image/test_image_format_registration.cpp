// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_writer.hpp"

#include <rexlib/core/service_catalog.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <em/image/formats/image_format_registration.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// Registration happens before main runs and is global: a format registered
// here is drained into every manager that asks for the builtin backends,
// which includes every manager a service_catalog hands out anywhere in this
// binary. So these two claim one extension nothing else uses, and decline
// everything else, which the last case below pins.
const char *const test_extension = ".rexlibtest";
const char *const read_format_name = "test-read";
const char *const write_format_name = "test-write";

bool claims(const image_probe &probe)
{
	return probe.get_extension() == test_extension;
}

// A real type rather than a mock, because a mock cannot register itself
// before main runs. It does the least a format can do.
class registered_read_format final
	: public image_read_format
{
public:
	std::string get_name() const override
	{
		return read_format_name;
	}

	backend_priority
	get_suitability(const image_probe &probe) const override
	{
		return claims(probe)
			? backend_priority::normal
			: backend_priority::unsupported;
	}

	std::unique_ptr<image_reader> open(const image_probe &) const override
	{
		return std::make_unique<mock_image_reader>();
	}
};

class registered_write_format final
	: public image_write_format
{
public:
	std::string get_name() const override
	{
		return write_format_name;
	}

	backend_priority
	get_suitability(const image_probe &probe) const override
	{
		return claims(probe)
			? backend_priority::normal
			: backend_priority::unsupported;
	}

	std::unique_ptr<image_writer> open(
		const image_probe &,
		span<const std::size_t>,
		std::size_t,
		numerical_type,
		const image_metadata &
	) const override
	{
		return std::make_unique<mock_image_writer>();
	}
};

std::string test_file()
{
	return std::string("absent") + test_extension;
}

} // namespace

REXLIB_REGISTER_IMAGE_READ_FORMAT(test, registered_read_format);
REXLIB_REGISTER_IMAGE_WRITE_FORMAT(test, registered_write_format);

TEST_CASE( "a read format registers itself at static initialization",
	"[image_format_registration]" )
{
	image_read_format_manager manager;

	SECTION( "it is absent until the builtin formats are drained" )
	{
		REQUIRE( manager.get_most_suitable_format(
			image_probe(test_file())) == nullptr );
	}

	SECTION( "draining the builtin formats reaches it" )
	{
		manager.register_builtin_backends();

		const auto *chosen =
			manager.get_most_suitable_format(image_probe(test_file()));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == read_format_name );
	}

	SECTION( "the drained format opens a reader" )
	{
		manager.register_builtin_backends();

		REQUIRE( manager.open(test_file()) != nullptr );
	}
}

TEST_CASE( "a write format registers itself at static initialization",
	"[image_format_registration]" )
{
	image_write_format_manager manager;
	const std::vector<std::size_t> extents = {2, 3, 4};

	SECTION( "it is absent until the builtin formats are drained" )
	{
		REQUIRE( manager.get_most_suitable_format(
			image_probe(test_file())) == nullptr );
	}

	SECTION( "draining the builtin formats reaches it" )
	{
		manager.register_builtin_backends();

		const auto *chosen =
			manager.get_most_suitable_format(image_probe(test_file()));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == write_format_name );
	}

	SECTION( "the drained format creates a writer" )
	{
		manager.register_builtin_backends();

		REQUIRE( manager.open(
			test_file(),
			make_span(extents),
			2,
			numerical_type::int16,
			image_metadata()
		) != nullptr );
	}

	SECTION( "reading and writing are registered separately" )
	{
		// The write format is drained into the write manager alone, so a
		// read manager never sees it and vice versa.
		manager.register_builtin_backends();

		REQUIRE( manager.get_most_suitable_format(
			image_probe(test_file()))->get_name() == write_format_name );
	}
}

TEST_CASE( "a registered format is inert for every other file",
	"[image_format_registration]" )
{
	// The guard on the hazard these registrations create. They are global,
	// so any test in this binary that drains the builtin backends — every
	// test that takes a manager from a service_catalog does — has them
	// registered. That is harmless only for as long as they claim nothing
	// but their own extension, so this says it outright.
	service_catalog catalog;
	const auto readers =
		catalog.get_service_manager<image_read_format_manager>();
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();

	const std::vector<std::string> others = {
		"absent.mrc", "absent.mrcs", "absent.tif", "absent.tiff",
		"absent.h5", "absent.em", "absent.eer", "absent", "absent."
	};

	for (const auto &path : others)
	{
		REQUIRE( readers->get_most_suitable_format(
			image_probe(path)) == nullptr );
		REQUIRE( writers->get_most_suitable_format(
			image_probe(path)) == nullptr );
	}
}
