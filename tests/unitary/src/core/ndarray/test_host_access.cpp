// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/ndarray/host_access.hpp>

#include "../hardware/mock/mock_buffer.hpp"
#include "../hardware/mock/mock_command_timeline.hpp"
#include "../hardware/mock/mock_memory_resource.hpp"

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/access_hazard_tracker.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;

namespace
{

array make_array(std::shared_ptr<buffer> storage)
{
	const std::vector<std::size_t> extents = {2, 2};

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32
		)
	);
}

class host_array_fixture
{
public:
	host_array_fixture()
		: values(4, 0.0F)
		, storage(std::make_shared<mock_buffer>())
		, subject(make_array(storage))
	{
		expectations.push_back(
			NAMED_ALLOW_CALL(resource, get_kind())
				.RETURN(memory_resource_kind::host)
		);
		expectations.push_back(
			NAMED_ALLOW_CALL(*storage, get_memory_resource())
				.LR_RETURN(std::ref(resource))
		);
		expectations.push_back(
			NAMED_ALLOW_CALL(*storage, get_host_ptr())
				.LR_RETURN(static_cast<void*>(values.data()))
		);

		// const_array_ref reaches the storage as const, which is the other
		// overload and so a separate expectation.
		const auto &readable = *storage;
		expectations.push_back(
			NAMED_ALLOW_CALL(readable, get_host_ptr())
				.LR_RETURN(static_cast<const void*>(values.data()))
		);
	}

protected:
	mock_memory_resource resource;
	std::vector<float> values;
	std::shared_ptr<mock_buffer> storage;
	array subject;
	std::vector<std::unique_ptr<trompeloeil::expectation>> expectations;
};

} // anonymous namespace

TEST_CASE_METHOD(
	host_array_fixture,
	"get_host_data yields the storage of an array the host can reach",
	"[host_access]"
)
{
	SECTION( "to write it" )
	{
		CHECK( get_host_data(array_ref(subject)) == values.data() );
	}

	SECTION( "to read it" )
	{
		CHECK( get_host_data(const_array_ref(subject)) == values.data() );
	}
}

TEST_CASE_METHOD(
	host_array_fixture,
	"get_host_data waits for the commands that conflict with the access",
	"[host_access]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	auto &tracker = *subject.get_access_hazard_tracker();
	tracker.add(command_token(timeline, 1), write_only);
	tracker.add(command_token(timeline, 2), read_only);

	SECTION( "to write, for the commands that read or write the array" )
	{
		REQUIRE_CALL(*timeline, wait(1u));
		REQUIRE_CALL(*timeline, wait(2u));
		CHECK( get_host_data(array_ref(subject)) == values.data() );
	}

	SECTION( "to read, for the command that writes the array" )
	{
		REQUIRE_CALL(*timeline, wait(1u));
		CHECK( get_host_data(const_array_ref(subject)) == values.data() );
	}
}

TEST_CASE(
	"get_host_data refuses an array with no storage",
	"[host_access]"
)
{
	SECTION( "to write it" )
	{
		CHECK_THROWS_AS(
			get_host_data(array_ref()),
			std::invalid_argument
		);
	}

	SECTION( "to read it" )
	{
		CHECK_THROWS_AS(
			get_host_data(const_array_ref()),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"get_host_data refuses an array the host cannot reach, without waiting",
	"[host_access]"
)
{
	mock_memory_resource resource;
	ALLOW_CALL(resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	auto storage = std::make_shared<mock_buffer>();
	ALLOW_CALL(*storage, get_memory_resource())
		.LR_RETURN(std::ref(resource));

	auto subject = make_array(storage);

	// No wait is expected on the timeline: reaching it would fail the test.
	const auto timeline = std::make_shared<mock_command_timeline>();
	subject.get_access_hazard_tracker()->add(
		command_token(timeline, 1),
		write_only
	);

	SECTION( "to write it" )
	{
		CHECK_THROWS_AS(
			get_host_data(array_ref(subject)),
			unsupported_capability_error
		);
	}

	SECTION( "to read it" )
	{
		CHECK_THROWS_AS(
			get_host_data(const_array_ref(subject)),
			unsupported_capability_error
		);
	}
}

TEST_CASE(
	"get_host_data refuses an array that exposes nothing to the host",
	"[host_access]"
)
{
	mock_memory_resource resource;
	ALLOW_CALL(resource, get_kind())
		.RETURN(memory_resource_kind::host);

	auto storage = std::make_shared<mock_buffer>();
	ALLOW_CALL(*storage, get_memory_resource())
		.LR_RETURN(std::ref(resource));
	ALLOW_CALL(*storage, get_host_ptr())
		.RETURN(nullptr);
	const auto &readable = *storage;
	ALLOW_CALL(readable, get_host_ptr())
		.RETURN(nullptr);

	auto subject = make_array(storage);

	SECTION( "to write it" )
	{
		CHECK_THROWS_AS(
			get_host_data(array_ref(subject)),
			unsupported_capability_error
		);
	}

	SECTION( "to read it" )
	{
		CHECK_THROWS_AS(
			get_host_data(const_array_ref(subject)),
			unsupported_capability_error
		);
	}
}
