// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/strided_transfer/image_host_access.hpp>

#include "../../../core/hardware/mock/mock_buffer.hpp"
#include "../../../core/hardware/mock/mock_memory_resource.hpp"

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

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

} // anonymous namespace

TEST_CASE( "an array the host can reach yields its storage",
	"[image_host_access]" )
{
	mock_memory_resource resource;
	ALLOW_CALL(resource, get_kind())
		.RETURN(memory_resource_kind::host);

	std::vector<float> values(4, 0.0F);
	auto storage = std::make_shared<mock_buffer>();
	ALLOW_CALL(*storage, get_memory_resource())
		.LR_RETURN(std::ref(resource));
	ALLOW_CALL(*storage, get_host_ptr())
		.LR_RETURN(static_cast<void*>(values.data()));

	// const_array_ref reaches the storage as const, which is the other
	// overload and so a separate expectation.
	const auto &readable = *storage;
	ALLOW_CALL(readable, get_host_ptr())
		.LR_RETURN(static_cast<const void*>(values.data()));

	auto subject = make_array(storage);

	SECTION( "a writable array yields where it holds them" )
	{
		REQUIRE( get_host_data(array_ref(subject)) == values.data() );
	}

	SECTION( "a readable array yields the same" )
	{
		REQUIRE( get_host_data(const_array_ref(subject)) == values.data() );
	}
}

TEST_CASE( "an array with no storage is refused", "[image_host_access]" )
{
	// An uninitialized array is not one anything can be moved into, and says
	// so as a contract violation rather than as something the format cannot
	// do.
	SECTION( "a writable one is refused" )
	{
		REQUIRE_THROWS_AS(
			get_host_data(array_ref()),
			std::invalid_argument
		);
	}

	SECTION( "a readable one is refused" )
	{
		REQUIRE_THROWS_AS(
			get_host_data(const_array_ref()),
			std::invalid_argument
		);
	}
}

TEST_CASE( "an array the host cannot reach is refused",
	"[image_host_access]" )
{
	// A transfer reads and writes the values of an array directly, so one
	// living on a device is not something this format can move anything into,
	// however well formed it is.
	mock_memory_resource resource;
	ALLOW_CALL(resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	auto storage = std::make_shared<mock_buffer>();
	ALLOW_CALL(*storage, get_memory_resource())
		.LR_RETURN(std::ref(resource));

	auto subject = make_array(storage);

	SECTION( "a writable one is refused" )
	{
		REQUIRE_THROWS_AS(
			get_host_data(array_ref(subject)),
			unsupported_capability_error
		);
	}

	SECTION( "a readable one is refused" )
	{
		REQUIRE_THROWS_AS(
			get_host_data(const_array_ref(subject)),
			unsupported_capability_error
		);
	}
}

TEST_CASE( "an array that exposes nothing to the host is refused",
	"[image_host_access]" )
{
	// Host accessible storage that hands back nothing is a different failure
	// from storage on a device, and neither is a contract violation by the
	// caller.
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

	REQUIRE_THROWS_AS(
		get_host_data(array_ref(subject)),
		unsupported_capability_error
	);
}
