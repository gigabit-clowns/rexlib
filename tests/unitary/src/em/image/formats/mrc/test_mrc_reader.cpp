// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/mrc/mrc_reader.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/mrc_test_file.hpp"

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;
using namespace rexlib::test;

namespace
{

std::size_t element_count(const std::vector<std::size_t> &extents)
{
	return std::accumulate(
		extents.cbegin(),
		extents.cend(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

array make_host_array(const std::vector<std::size_t> &extents)
{
	auto storage = std::make_shared<host_buffer>(
		element_count(extents) * sizeof(float),
		alignof(float)
	);

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32
		)
	);
}

std::vector<float> values_of(
	const array &destination,
	const std::vector<std::size_t> &extents
)
{
	const auto *data = static_cast<const float*>(
		destination.get_storage()->get_host_ptr());

	return std::vector<float>(data, data + element_count(extents));
}

// Every file the cases below build holds float32, mode 2.
image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank
)
{
	return image_descriptor(
		make_span(extents),
		core_rank,
		numerical_type::float32
	);
}

std::vector<float> read_all(const mrc_reader &reader)
{
	const auto extents = reader.get_descriptor().get_extents();
	const std::vector<std::size_t> shape(extents.begin(), extents.end());

	auto destination = make_host_array(shape);

	image_transfer_plan regions(
		make_span(shape), shape.size(), shape.size());
	regions.add(
		make_span(std::vector<std::size_t>(shape.size(), 0)),
		make_span(std::vector<std::size_t>(shape.size(), 0))
	);

	reader.read(array_ref(destination), regions);

	return values_of(destination, shape);
}

} // anonymous namespace

TEST_CASE( "an MRC file is opened and reports what it holds",
	"[mrc_reader]" )
{
	const scoped_path path("reader_open.mrc");

	SECTION( "a stack of images reports two core axes" )
	{
		write_file(path.get(), make_file(4, 3, 2, 0, 2, counting(24)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor() == make_descriptor({2, 3, 4}, 2) );
	}

	SECTION( "a volume reports three" )
	{
		write_file(path.get(), make_file(4, 3, 2, 1, 2, counting(24)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor() == make_descriptor({2, 3, 4}, 3) );
	}

	SECTION( "a single image drops the section axis" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor() == make_descriptor({3, 4}, 2) );
	}

	SECTION( "a stack of volumes one section deep is still one" )
	{
		auto raw = make_file(4, 3, 6, 401, 2, counting(72));
		put_int32(raw, 36, 1);
		write_file(path.get(), raw);

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor() ==
			make_descriptor({6, 1, 3, 4}, 3) );
	}

	SECTION( "a stack of a single volume is still one" )
	{
		write_file(path.get(), make_file(4, 3, 6, 401, 2, counting(72)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor() ==
			make_descriptor({1, 6, 3, 4}, 3) );
	}

	SECTION( "a single image read as a stack is a stack of one" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const mrc_reader reader(path.get(), mrc_single_section::image_stack);

		REQUIRE( reader.get_descriptor() == make_descriptor({1, 3, 4}, 2) );
		REQUIRE( read_all(reader) == counting(12) );
	}
}

TEST_CASE( "the values of an MRC file are read into an array",
	"[mrc_reader]" )
{
	const scoped_path path("reader_values.mrc");

	SECTION( "the whole of a stack arrives in order" )
	{
		const auto values = counting(24);
		write_file(path.get(), make_file(4, 3, 2, 0, 2, values));

		const mrc_reader reader(path.get());

		REQUIRE( read_all(reader) == values );
	}

	SECTION( "one plane of a stack is read on its own" )
	{
		const auto values = counting(24);
		write_file(path.get(), make_file(4, 3, 2, 0, 2, values));

		const mrc_reader reader(path.get());

		const std::vector<std::size_t> region = {3, 4};
		auto destination = make_host_array(region);

		image_transfer_plan regions(make_span(region), 3, 2);
		regions.add(
			make_span(std::vector<std::size_t>{1, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		reader.read(array_ref(destination), regions);

		REQUIRE( values_of(destination, region) ==
			std::vector<float>(values.begin() + 12, values.end()) );
	}

	SECTION( "regions stated out of order all arrive where they belong" )
	{
		// The reader orders the regions by their place in the file before it
		// walks them, so this is what guards that each one keeps the slot of
		// the array it was stated with.
		const auto values = counting(36);
		write_file(path.get(), make_file(4, 3, 3, 0, 2, values));

		const mrc_reader reader(path.get());

		const std::vector<std::size_t> shape = {3, 3, 4};
		auto destination = make_host_array(shape);

		const std::vector<std::size_t> region = {3, 4};
		image_transfer_plan regions(make_span(region), 3, 3);

		const std::size_t sections[3] = {2, 0, 1};
		for (std::size_t i = 0; i < 3; ++i)
		{
			regions.add(
				make_span(std::vector<std::size_t>{sections[i], 0, 0}),
				make_span(std::vector<std::size_t>{i, 0, 0})
			);
		}

		reader.read(array_ref(destination), regions);

		std::vector<float> expected;
		for (const auto section : sections)
		{
			expected.insert(
				expected.end(),
				values.begin() + static_cast<std::ptrdiff_t>(section * 12),
				values.begin() + static_cast<std::ptrdiff_t>(section * 12 + 12)
			);
		}

		REQUIRE( values_of(destination, shape) == expected );
	}

	SECTION( "an empty plan reads nothing and succeeds" )
	{
		write_file(path.get(), make_file(4, 3, 2, 0, 2, counting(24)));

		const mrc_reader reader(path.get());

		const std::vector<std::size_t> region = {3, 4};
		auto destination = make_host_array(region);
		const image_transfer_plan regions(make_span(region), 3, 2);

		REQUIRE_NOTHROW( reader.read(array_ref(destination), regions) );
	}

	SECTION( "an uninitialized destination is refused" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const mrc_reader reader(path.get());
		const std::vector<std::size_t> region = {3, 4};
		image_transfer_plan regions(make_span(region), 2, 2);

		REQUIRE_THROWS_MATCHES(
			reader.read(array_ref(), regions),
			std::invalid_argument,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get() + ": ")
			)
		);
	}
}

TEST_CASE( "a file that contradicts its own header is refused",
	"[mrc_reader]" )
{
	const scoped_path path("reader_bad_header.mrc");
	const auto names_the_file = Catch::Matchers::MessageMatches(
		Catch::Matchers::StartsWith(path.get() + ": ")
	);

	SECTION( "one shorter than the shape it states is refused" )
	{
		auto raw = make_file(4, 3, 2, 0, 2, counting(24));
		raw.resize(raw.size() - sizeof(float));
		write_file(path.get(), raw);

		REQUIRE_THROWS_MATCHES(
			mrc_reader(path.get()),
			image_format_error,
			names_the_file
		);
	}

	SECTION( "one without the identifier is refused" )
	{
		auto raw = make_file(4, 3, 1, 0, 2, counting(12));
		raw[208] = 'X';
		write_file(path.get(), raw);

		REQUIRE_THROWS_MATCHES(
			mrc_reader(path.get()),
			image_format_error,
			names_the_file
		);
	}

	SECTION( "one that is not there is refused" )
	{
		REQUIRE_THROWS_MATCHES(
			mrc_reader(path.get()),
			image_file_error,
			names_the_file
		);
	}
}
