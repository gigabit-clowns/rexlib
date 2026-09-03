// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/managed_image_writer_provider.hpp>

#include "fake/fake_image_writer.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> stack_extents = {4, 3, 5};

// Records every file it is asked to create, so that a test can say how many
// times each was created. The manager is final, so counting happens here.
class counting_format final
	: public image_write_format
{
public:
	explicit counting_format(std::shared_ptr<std::vector<std::string>> log)
		: m_log(std::move(log))
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

	std::unique_ptr<image_writer> open(
		const image_probe &probe,
		span<const std::size_t> extents,
		std::size_t core_rank,
		numerical_type,
		const image_metadata &
	) const override
	{
		m_log->push_back(probe.get_path());
		return std::unique_ptr<image_writer>(
			new fake_image_writer(extents, core_rank)
		);
	}

private:
	std::shared_ptr<std::vector<std::string>> m_log;
};

std::shared_ptr<const image_write_format_manager> make_manager(
	const std::shared_ptr<std::vector<std::string>> &log
)
{
	auto manager = std::make_shared<image_write_format_manager>();
	manager->register_format(
		std::unique_ptr<image_write_format>(new counting_format(log))
	);
	return manager;
}

void declare_stack(
	managed_image_writer_provider &provider,
	std::string path
)
{
	provider.declare(
		std::move(path),
		make_span(stack_extents),
		2,
		numerical_type::int16,
		image_metadata()
	);
}

std::size_t count(
	const std::vector<std::string> &log,
	const std::string &path
)
{
	std::size_t result = 0;
	for (const auto &entry : log)
	{
		if (entry == path)
		{
			++result;
		}
	}
	return result;
}

} // namespace

TEST_CASE( "a managed writer provider needs a format manager",
	"[managed_image_writer_provider]" )
{
	REQUIRE_THROWS_AS(
		managed_image_writer_provider(nullptr),
		std::invalid_argument
	);
}

TEST_CASE( "a managed writer provider serves only what was declared",
	"[managed_image_writer_provider]" )
{
	const auto log = std::make_shared<std::vector<std::string>>();
	managed_image_writer_provider provider(make_manager(log));

	SECTION( "it starts serving nothing" )
	{
		REQUIRE( provider.get_size() == 0 );
		REQUIRE_THROWS_AS(
			provider.acquire("stack_0.mrcs"),
			std::out_of_range
		);
	}

	SECTION( "declaring costs no file" )
	{
		declare_stack(provider, "stack_0.mrcs");

		REQUIRE( provider.get_size() == 1 );
		REQUIRE( log->empty() );
	}

	SECTION( "a declared path is refused a second time" )
	{
		// Replacing it would strand whatever had been written to the file
		// it names, so the caller has to close it and say so.
		declare_stack(provider, "stack_0.mrcs");

		REQUIRE_THROWS_AS(
			declare_stack(provider, "stack_0.mrcs"),
			invalid_operation_error
		);
		REQUIRE( provider.get_size() == 1 );
	}
}

TEST_CASE( "a managed writer provider refuses a core rank it can not mean",
	"[managed_image_writer_provider]" )
{
	// Refused where it was written down rather than at whatever later point
	// the file is first acquired.
	const auto log = std::make_shared<std::vector<std::string>>();
	managed_image_writer_provider provider(make_manager(log));

	SECTION( "a core rank of zero names no image or volume" )
	{
		REQUIRE_THROWS_AS(
			provider.declare(
				"stack_0.mrcs",
				make_span(stack_extents),
				0,
				numerical_type::int16,
				image_metadata()
			),
			std::invalid_argument
		);
		REQUIRE( provider.get_size() == 0 );
	}

	SECTION( "a core rank above the rank of the extents is refused" )
	{
		REQUIRE_THROWS_AS(
			provider.declare(
				"stack_0.mrcs",
				make_span(stack_extents),
				4,
				numerical_type::int16,
				image_metadata()
			),
			std::invalid_argument
		);
		REQUIRE( provider.get_size() == 0 );
	}

	SECTION( "a core rank equal to the rank of the extents is one volume" )
	{
		provider.declare(
			"volume.mrc",
			make_span(stack_extents),
			3,
			numerical_type::float32,
			image_metadata()
		);

		REQUIRE( provider.acquire("volume.mrc")->get_core_rank() == 3 );
	}
}

TEST_CASE( "a managed writer provider creates a file once",
	"[managed_image_writer_provider]" )
{
	const auto log = std::make_shared<std::vector<std::string>>();
	managed_image_writer_provider provider(make_manager(log));
	declare_stack(provider, "stack_0.mrcs");

	SECTION( "the first acquire creates it" )
	{
		const auto writer = provider.acquire("stack_0.mrcs");

		REQUIRE( writer != nullptr );
		REQUIRE( count(*log, "stack_0.mrcs") == 1 );
	}

	SECTION( "every later acquire yields the same writer" )
	{
		// Creating it again would replace the file, so it must not happen.
		const auto first = provider.acquire("stack_0.mrcs");
		const auto second = provider.acquire("stack_0.mrcs");

		REQUIRE( first == second );
		REQUIRE( count(*log, "stack_0.mrcs") == 1 );
	}

	SECTION( "it creates it with what was declared" )
	{
		const auto writer = provider.acquire("stack_0.mrcs");
		const auto extents = writer->get_extents();

		REQUIRE( std::vector<std::size_t>(extents.begin(), extents.end()) ==
			stack_extents );
		REQUIRE( writer->get_core_rank() == 2 );
	}
}

TEST_CASE( "closing a file finishes it", "[managed_image_writer_provider]" )
{
	const auto log = std::make_shared<std::vector<std::string>>();
	managed_image_writer_provider provider(make_manager(log));
	declare_stack(provider, "stack_0.mrcs");

	SECTION( "closing what was never declared is refused" )
	{
		REQUIRE_THROWS_AS( provider.close("absent.mrcs"), std::out_of_range );
	}

	SECTION( "close flushes the writer it drops" )
	{
		const auto writer = provider.acquire("stack_0.mrcs");
		const auto *fake = static_cast<const fake_image_writer*>(
			writer.get()
		);
		REQUIRE( fake->get_flush_count() == 0 );

		provider.close("stack_0.mrcs");

		REQUIRE( fake->get_flush_count() == 1 );
	}

	SECTION( "closing forgets the declaration, not just the handle" )
	{
		// A handle-only close would let the next acquire replace the file
		// and strand everything already written to it.
		provider.acquire("stack_0.mrcs");
		provider.close("stack_0.mrcs");

		REQUIRE( provider.get_size() == 0 );
		REQUIRE_THROWS_AS(
			provider.acquire("stack_0.mrcs"),
			std::out_of_range
		);
	}

	SECTION( "closing a file never acquired creates nothing" )
	{
		provider.close("stack_0.mrcs");

		REQUIRE( provider.get_size() == 0 );
		REQUIRE( log->empty() );
	}

	SECTION( "declaring again after a close creates the file afresh" )
	{
		provider.acquire("stack_0.mrcs");
		provider.close("stack_0.mrcs");
		declare_stack(provider, "stack_0.mrcs");
		provider.acquire("stack_0.mrcs");

		REQUIRE( count(*log, "stack_0.mrcs") == 2 );
	}
}

TEST_CASE( "a managed writer provider flushes what it opened and no more",
	"[managed_image_writer_provider]" )
{
	const auto log = std::make_shared<std::vector<std::string>>();
	managed_image_writer_provider provider(make_manager(log));
	declare_stack(provider, "stack_0.mrcs");
	declare_stack(provider, "stack_1.mrcs");
	declare_stack(provider, "stack_2.mrcs");

	const auto zero = provider.acquire("stack_0.mrcs");
	const auto one = provider.acquire("stack_1.mrcs");

	provider.flush();

	SECTION( "every writer it created is flushed" )
	{
		REQUIRE(
			static_cast<const fake_image_writer*>(zero.get())
				->get_flush_count() == 1
		);
		REQUIRE(
			static_cast<const fake_image_writer*>(one.get())
				->get_flush_count() == 1
		);
	}

	SECTION( "a declared file that was never acquired is not created" )
	{
		REQUIRE( count(*log, "stack_2.mrcs") == 0 );
		REQUIRE( provider.get_size() == 3 );
	}
}
