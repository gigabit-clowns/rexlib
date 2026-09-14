// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_reader.hpp>

#include <em/image/formats/mrc/mrc_constants.hpp>
#include <em/image/formats/mrc/mrc_read_format.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/tests/assets.hpp>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

// A path under the build tree that is removed when the test leaves, whether
// it succeeded or not.
//
// The scratch directory is shared and every case runs as a process of its
// own, so two cases naming the same file race: one truncates what the other
// has mapped. No two names here may repeat.
class scoped_path
{
public:
	explicit scoped_path(const std::string &name)
		: m_path(get_scratch_path(name))
	{
		std::remove(m_path.c_str());
	}

	scoped_path(const scoped_path &other) = delete;
	scoped_path(scoped_path &&other) = delete;

	~scoped_path()
	{
		std::remove(m_path.c_str());
	}

	scoped_path& operator=(const scoped_path &other) = delete;
	scoped_path& operator=(scoped_path &&other) = delete;

	const std::string& get() const noexcept
	{
		return m_path;
	}

private:
	std::string m_path;
};

void put_int32(std::vector<char> &raw, std::size_t offset, std::int32_t value)
{
	std::memcpy(raw.data() + offset, &value, sizeof(value));
}

// A little-endian MRC file, laid out for a host of that byte order, which is
// what every machine the suite runs on is.
std::vector<char> make_file(
	std::int32_t columns,
	std::int32_t rows,
	std::int32_t sections,
	std::int32_t space_group,
	std::int32_t mode,
	const std::vector<float> &values
)
{
	std::vector<char> raw(header_size + values.size() * sizeof(float), '\0');

	put_int32(raw, 0, columns);
	put_int32(raw, 4, rows);
	put_int32(raw, 8, sections);
	put_int32(raw, 12, mode);
	put_int32(raw, 36, space_group == 0 ? 1 : sections);
	put_int32(raw, 64, 1);
	put_int32(raw, 68, 2);
	put_int32(raw, 72, 3);
	put_int32(raw, 88, space_group);
	std::memcpy(raw.data() + 208, "MAP ", 4);
	raw[212] = static_cast<char>(0x44);
	raw[213] = static_cast<char>(0x44);

	std::memcpy(
		raw.data() + header_size,
		values.data(),
		values.size() * sizeof(float)
	);

	return raw;
}

void write_file(const std::string &path, const std::vector<char> &raw)
{
	std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
	output.write(raw.data(), static_cast<std::streamsize>(raw.size()));
}

std::vector<float> counting(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(i);
	}

	return values;
}

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

std::vector<float> read_all(const mrc_reader &reader)
{
	const auto extents = reader.get_extents();
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

		REQUIRE( reader.get_extents().size() == 3 );
		REQUIRE( reader.get_extents()[0] == 2 );
		REQUIRE( reader.get_extents()[1] == 3 );
		REQUIRE( reader.get_extents()[2] == 4 );
		REQUIRE( reader.get_core_rank() == 2 );
		REQUIRE( reader.get_data_type() == numerical_type::float32 );
	}

	SECTION( "a volume reports three" )
	{
		write_file(path.get(), make_file(4, 3, 2, 1, 2, counting(24)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_core_rank() == 3 );
	}

	SECTION( "a single image drops the section axis" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_extents().size() == 2 );
		REQUIRE( reader.get_core_rank() == 2 );
	}

	SECTION( "a stack of volumes one section thick is a stack of images" )
	{
		auto raw = make_file(4, 3, 6, 401, 2, counting(72));
		put_int32(raw, 36, 1);
		write_file(path.get(), raw);

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_extents().size() == 3 );
		REQUIRE( reader.get_extents()[0] == 6 );
		REQUIRE( reader.get_extents()[1] == 3 );
		REQUIRE( reader.get_extents()[2] == 4 );
		REQUIRE( reader.get_core_rank() == 2 );
	}

	SECTION( "a stack of a single volume is a volume" )
	{
		write_file(path.get(), make_file(4, 3, 6, 401, 2, counting(72)));

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_extents().size() == 3 );
		REQUIRE( reader.get_extents()[0] == 6 );
		REQUIRE( reader.get_core_rank() == 3 );
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

		REQUIRE_THROWS_AS(
			reader.read(array_ref(), regions),
			std::invalid_argument
		);
	}
}

TEST_CASE( "a file that contradicts its own header is refused",
	"[mrc_reader]" )
{
	const scoped_path path("reader_bad_header.mrc");

	SECTION( "one shorter than the shape it states is refused" )
	{
		auto raw = make_file(4, 3, 2, 0, 2, counting(24));
		raw.resize(raw.size() - sizeof(float));
		write_file(path.get(), raw);

		REQUIRE_THROWS_AS( mrc_reader(path.get()), image_format_error );
	}

	SECTION( "one without the identifier is refused" )
	{
		auto raw = make_file(4, 3, 1, 0, 2, counting(12));
		raw[208] = 'X';
		write_file(path.get(), raw);

		REQUIRE_THROWS_AS( mrc_reader(path.get()), image_format_error );
	}

	SECTION( "one that is not there is refused" )
	{
		REQUIRE_THROWS_AS( mrc_reader(path.get()), image_format_error );
	}
}

TEST_CASE( "the MRC format claims the files it can read",
	"[mrc_read_format]" )
{
	const scoped_path path("reader_claimed.mrc");
	const mrc_read_format format;

	SECTION( "it is named" )
	{
		REQUIRE( format.get_name() == "MRC" );
	}

	SECTION( "a file carrying the identifier is claimed" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::normal );
	}

	SECTION( "the identifier is claimed whatever the extension" )
	{
		const std::string other = path.get() + ".unknown";
		write_file(other, make_file(4, 3, 1, 0, 2, counting(12)));

		REQUIRE( format.get_suitability(image_probe(other)) ==
			backend_priority::normal );

		std::remove(other.c_str());
	}

	SECTION( "a long enough file of a known extension is claimed weakly" )
	{
		auto raw = make_file(4, 3, 1, 0, 2, counting(12));
		std::memcpy(raw.data() + 208, "\0\0\0\0", 4);
		write_file(path.get(), raw);

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::fallback );
	}

	SECTION( "a file too short to hold a header is not claimed" )
	{
		write_file(path.get(), std::vector<char>(16, '\0'));

		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::unsupported );
	}

	SECTION( "a file that is not there is not claimed" )
	{
		REQUIRE( format.get_suitability(image_probe(path.get())) ==
			backend_priority::unsupported );
	}

	SECTION( "a claimed file opens into a reader" )
	{
		write_file(path.get(), make_file(4, 3, 1, 0, 2, counting(12)));

		const auto reader = format.open(image_probe(path.get()));

		REQUIRE( reader != nullptr );
		REQUIRE( reader->get_core_rank() == 2 );
	}
}
