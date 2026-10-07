// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_header.hpp>

#include <em/image/formats/mrc/mrc_constants.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <array>
#include <cstring>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

// The offsets are restated here rather than taken from mrc_constants.hpp, so
// that a test written against the specification does not agree with the parser
// merely by sharing its table.
REXLIB_CONST_CONSTEXPR std::size_t nx_offset = 0;
REXLIB_CONST_CONSTEXPR std::size_t ny_offset = 4;
REXLIB_CONST_CONSTEXPR std::size_t nz_offset = 8;
REXLIB_CONST_CONSTEXPR std::size_t mode_offset = 12;
REXLIB_CONST_CONSTEXPR std::size_t mx_offset = 28;
REXLIB_CONST_CONSTEXPR std::size_t mz_offset = 36;
REXLIB_CONST_CONSTEXPR std::size_t cella_offset = 40;
REXLIB_CONST_CONSTEXPR std::size_t mapc_offset = 64;
REXLIB_CONST_CONSTEXPR std::size_t mapr_offset = 68;
REXLIB_CONST_CONSTEXPR std::size_t maps_offset = 72;
REXLIB_CONST_CONSTEXPR std::size_t ispg_offset = 88;
REXLIB_CONST_CONSTEXPR std::size_t nsymbt_offset = 92;
REXLIB_CONST_CONSTEXPR std::size_t imod_stamp_offset = 152;
REXLIB_CONST_CONSTEXPR std::size_t imod_flags_offset = 156;
REXLIB_CONST_CONSTEXPR std::size_t nlabl_offset = 220;
REXLIB_CONST_CONSTEXPR std::size_t label_offset = 224;
REXLIB_CONST_CONSTEXPR std::size_t label_size = 80;
REXLIB_CONST_CONSTEXPR std::size_t map_offset = 208;
REXLIB_CONST_CONSTEXPR std::size_t machst_offset = 212;

using raw_header = std::vector<rexlib::byte>;

void put_bytes(
	raw_header &raw,
	std::size_t offset,
	const void *value,
	std::size_t count,
	byte_order order
)
{
	const auto *source = static_cast<const unsigned char*>(value);
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto position = order == byte_order::little_endian
			? i
			: count - 1 - i;
		raw[offset + position] = as_byte(source[i]);
	}
}

void put_int32(
	raw_header &raw,
	std::size_t offset,
	std::int32_t value,
	byte_order order
)
{
	// The source bytes are least significant first only on a little-endian
	// host, so they are laid out from the value itself rather than copied.
	std::array<unsigned char, 4> source;
	for (std::size_t i = 0; i < source.size(); ++i)
	{
		source[i] = static_cast<unsigned char>(
			(static_cast<std::uint32_t>(value) >> (8 * i)) & 0xFFU
		);
	}

	put_bytes(raw, offset, source.data(), source.size(), order);
}

void put_float(
	raw_header &raw,
	std::size_t offset,
	float value,
	byte_order order
)
{
	std::uint32_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	put_int32(raw, offset, static_cast<std::int32_t>(bits), order);
}

void put_map_identifier(raw_header &raw)
{
	raw[map_offset] = as_byte('M');
	raw[map_offset + 1] = as_byte('A');
	raw[map_offset + 2] = as_byte('P');
	raw[map_offset + 3] = as_byte(' ');
}

void put_machine_stamp(
	raw_header &raw,
	std::uint8_t first,
	std::uint8_t second
)
{
	raw[machst_offset] = as_byte(first);
	raw[machst_offset + 1] = as_byte(second);
}

raw_header make_raw_header(byte_order order)
{
	raw_header raw(header_size, rexlib::byte{});

	put_int32(raw, nx_offset, 4, order);
	put_int32(raw, ny_offset, 3, order);
	put_int32(raw, nz_offset, 2, order);
	put_int32(raw, mode_offset, 2, order);
	put_int32(raw, mx_offset, 4, order);
	put_int32(raw, mapc_offset, 1, order);
	put_int32(raw, mapr_offset, 2, order);
	put_int32(raw, maps_offset, 3, order);
	put_map_identifier(raw);
	put_machine_stamp(
		raw,
		order == byte_order::little_endian ? 0x44 : 0x11,
		order == byte_order::little_endian ? 0x44 : 0x11
	);

	return raw;
}

span<const rexlib::byte> view(const raw_header &raw)
{
	return make_span(raw.data(), raw.size());
}

} // anonymous namespace

TEST_CASE( "the MRC identifier is found where the specification puts it",
	"[mrc_header]" )
{
	SECTION( "a header carrying it is recognized" )
	{
		const auto raw = make_raw_header(byte_order::little_endian);

		REQUIRE( has_map_identifier(view(raw)) );
	}

	SECTION( "only its first three bytes have to match" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		raw[map_offset + 3] = as_byte('\0');

		REQUIRE( has_map_identifier(view(raw)) );
	}

	SECTION( "a header without it is not" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		raw[map_offset] = as_byte('X');

		REQUIRE_FALSE( has_map_identifier(view(raw)) );
	}

	SECTION( "bytes too few to reach it are not" )
	{
		const raw_header raw(map_offset, rexlib::byte{});

		REQUIRE_FALSE( has_map_identifier(view(raw)) );
	}
}

TEST_CASE( "an MRC header is parsed in the byte order its stamp names",
	"[mrc_header]" )
{
	SECTION( "a little-endian header is read" )
	{
		const auto raw = make_raw_header(byte_order::little_endian);
		const auto header = parse_header(view(raw));

		REQUIRE( header.get_byte_order() == byte_order::little_endian );
		REQUIRE( header.get_column_count() == 4 );
		REQUIRE( header.get_row_count() == 3 );
		REQUIRE( header.get_section_count() == 2 );
		REQUIRE( header.get_mode() == mrc_mode::float32 );
	}

	SECTION( "a big-endian header is read" )
	{
		const auto raw = make_raw_header(byte_order::big_endian);
		const auto header = parse_header(view(raw));

		REQUIRE( header.get_byte_order() == byte_order::big_endian );
		REQUIRE( header.get_column_count() == 4 );
		REQUIRE( header.get_row_count() == 3 );
		REQUIRE( header.get_section_count() == 2 );
		REQUIRE( header.get_mode() == mrc_mode::float32 );
	}

	SECTION( "the legacy little-endian stamp is accepted" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_machine_stamp(raw, 0x44, 0x41);

		const auto header = parse_header(view(raw));

		REQUIRE( header.get_byte_order() == byte_order::little_endian );
		REQUIRE( header.get_column_count() == 4 );
	}

	SECTION( "floating point fields cross the byte order too" )
	{
		auto raw = make_raw_header(byte_order::big_endian);
		put_float(raw, cella_offset, 1.5F, byte_order::big_endian);

		const auto header = parse_header(view(raw));

		REQUIRE( header.get_cell_size()[0] == 1.5F );
	}
}

TEST_CASE( "a stamp that contradicts the mode does not decide the byte order",
	"[mrc_header]" )
{
	SECTION( "a wrong stamp is overruled by the mode" )
	{
		auto raw = make_raw_header(byte_order::big_endian);
		put_machine_stamp(raw, 0x44, 0x44);

		const auto header = parse_header(view(raw));

		REQUIRE( header.get_byte_order() == byte_order::big_endian );
		REQUIRE( header.get_column_count() == 4 );
	}

	SECTION( "an unrecognized stamp is resolved by the mode" )
	{
		auto raw = make_raw_header(byte_order::big_endian);
		put_machine_stamp(raw, 0x00, 0x00);

		const auto header = parse_header(view(raw));

		REQUIRE( header.get_byte_order() == byte_order::big_endian );
		REQUIRE( header.get_column_count() == 4 );
	}

	SECTION( "a mode no byte order resolves is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, mode_offset, 3, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	SECTION( "the packed mode is refused as well" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, mode_offset, 101, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}
}

TEST_CASE( "an MRC header that contradicts the format is refused",
	"[mrc_header]" )
{
	SECTION( "a block shorter than the header is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		raw.resize(header_size - 1);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	SECTION( "a block without the identifier is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		raw[map_offset] = as_byte('X');

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	SECTION( "a negative extent is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, ny_offset, -3, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	SECTION( "a negative extended header size is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, nsymbt_offset, -8, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	SECTION( "an axis correspondence that repeats an axis is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, maps_offset, 1, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	// Zero is no axis, which is what a writer that never touched the three
	// fields leaves behind, and is read as the three axes in order.
	SECTION( "an axis correspondence of zeros is not refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, mapc_offset, 0, byte_order::little_endian);
		put_int32(raw, mapr_offset, 0, byte_order::little_endian);
		put_int32(raw, maps_offset, 0, byte_order::little_endian);

		const auto header = parse_header(view(raw));

		REQUIRE( has_unset_axes(header) );
		REQUIRE_FALSE( has_axis_permutation(header) );
	}

	SECTION( "a stack of volumes that does not divide evenly is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, ispg_offset, 401, byte_order::little_endian);
		put_int32(raw, nz_offset, 7, byte_order::little_endian);
		put_int32(raw, mz_offset, 2, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}

	SECTION( "a stack of volumes of no depth is refused" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, ispg_offset, 401, byte_order::little_endian);
		put_int32(raw, mz_offset, 0, byte_order::little_endian);

		REQUIRE_THROWS_AS( parse_header(view(raw)), image_format_error );
	}
}

TEST_CASE( "an MRC header states nothing about the version or the stamp form",
	"[mrc_header]" )
{
	SECTION( "a header of version zero is read" )
	{
		const auto raw = make_raw_header(byte_order::little_endian);
		const auto header = parse_header(view(raw));

		REQUIRE( header.get_version() == 0 );
	}

	SECTION( "the legacy stamp and a zero version parse together" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_machine_stamp(raw, 0x44, 0x41);

		REQUIRE_NOTHROW( parse_header(view(raw)) );
	}
}

TEST_CASE( "mode 0 takes its signedness from the IMOD stamp",
	"[mrc_header]" )
{
	auto raw = make_raw_header(byte_order::little_endian);
	put_int32(raw, mode_offset, 0, byte_order::little_endian);

	SECTION( "without the stamp it holds signed values" )
	{
		const auto header = parse_header(view(raw));

		REQUIRE( holds_signed_bytes(header) );
		REQUIRE( get_data_type(header) == numerical_type::int8 );
	}

	SECTION( "with the stamp and the flag it holds signed values" )
	{
		put_int32(
			raw, imod_stamp_offset, 1146047817, byte_order::little_endian);
		put_int32(raw, imod_flags_offset, 1, byte_order::little_endian);

		const auto header = parse_header(view(raw));

		REQUIRE( holds_signed_bytes(header) );
		REQUIRE( get_data_type(header) == numerical_type::int8 );
	}

	SECTION( "with the stamp and no flag it holds unsigned values" )
	{
		put_int32(
			raw, imod_stamp_offset, 1146047817, byte_order::little_endian);

		const auto header = parse_header(view(raw));

		REQUIRE_FALSE( holds_signed_bytes(header) );
		REQUIRE( get_data_type(header) == numerical_type::uint8 );
	}

	SECTION( "the flag without the stamp says nothing" )
	{
		put_int32(raw, imod_flags_offset, 0, byte_order::little_endian);

		const auto header = parse_header(view(raw));

		REQUIRE( holds_signed_bytes(header) );
		REQUIRE( get_data_type(header) == numerical_type::int8 );
	}
}

TEST_CASE( "the values of a file begin past both headers",
	"[mrc_header]" )
{
	SECTION( "without an extended header they follow the main one" )
	{
		const auto raw = make_raw_header(byte_order::little_endian);
		const auto header = parse_header(view(raw));

		REQUIRE( get_data_offset(header) == 1024 );
	}

	SECTION( "an extended header pushes them further" )
	{
		auto raw = make_raw_header(byte_order::little_endian);
		put_int32(raw, nsymbt_offset, 160, byte_order::little_endian);

		const auto header = parse_header(view(raw));

		REQUIRE( get_data_offset(header) == 1184 );
	}
}

TEST_CASE( "an MRC header survives being written and read back",
	"[mrc_header]" )
{
	SECTION( "a parsed header round-trips in each byte order" )
	{
		const std::array<byte_order, 2> orders = {
			byte_order::little_endian,
			byte_order::big_endian
		};

		for (const auto order : orders)
		{
			auto raw = make_raw_header(order);
			put_int32(raw, nsymbt_offset, 160, order);
			put_float(raw, cella_offset, 2.25F, order);

			const auto original = parse_header(view(raw));

			raw_header written(header_size, rexlib::byte{});
			serialize_header(original, make_span(written.data(),
				written.size()));
			const auto reread = parse_header(view(written));

			REQUIRE( reread.get_byte_order() == original.get_byte_order() );
			REQUIRE( reread.get_column_count() ==
				original.get_column_count() );
			REQUIRE( reread.get_row_count() == original.get_row_count() );
			REQUIRE( reread.get_section_count() ==
				original.get_section_count() );
			REQUIRE( reread.get_mode() == original.get_mode() );
			REQUIRE( reread.get_extended_header_size() ==
				original.get_extended_header_size() );
			REQUIRE( reread.get_cell_size() == original.get_cell_size() );
			REQUIRE( reread.get_space_group() ==
				original.get_space_group() );
		}
	}

	SECTION( "a written header carries the identifier" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));

		REQUIRE( has_map_identifier(view(written)) );
		REQUIRE( as_uint8(written[map_offset + 3]) ==
			static_cast<std::uint8_t>(' ') );
	}

	SECTION( "a written header stamps the byte order it states" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);
		header.set_byte_order(byte_order::big_endian);

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));

		REQUIRE( as_uint8(written[machst_offset]) == 0x11 );
		REQUIRE( as_uint8(written[machst_offset + 1]) == 0x11 );
	}

	SECTION( "the IMOD words survive a round trip" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::int8);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);
		header.set_imod_stamp(1146047817);

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));
		const auto reread = parse_header(view(written));

		REQUIRE( reread.get_imod_stamp() == 1146047817 );
		REQUIRE( reread.get_imod_flags() == 0 );
		REQUIRE_FALSE( holds_signed_bytes(reread) );
	}

	SECTION( "a destination too short is refused" )
	{
		const mrc_header header;
		raw_header written(header_size - 1, rexlib::byte{});

		REQUIRE_THROWS_AS(
			serialize_header(
				header, make_span(written.data(), written.size())),
			std::invalid_argument
		);
	}
}

TEST_CASE( "an MRC file records the software that wrote it",
	"[mrc_header]" )
{
	SECTION( "a label survives being written and read back" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);
		header.add_label("Created by something");

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));
		const auto reread = parse_header(view(written));

		REQUIRE( reread.get_labels().size() == 1 );
		REQUIRE( reread.get_labels()[0] == "Created by something" );
	}

	SECTION( "the count and the records agree on what is written" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);
		header.add_label("first");
		header.add_label("second");

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));

		std::int32_t stated = 0;
		std::memcpy(&stated, written.data() + nlabl_offset, sizeof(stated));

		REQUIRE( stated == 2 );
		REQUIRE( std::string(
			reinterpret_cast<const char*>(written.data()) + label_offset,
			5) == "first" );
		REQUIRE( std::string(
			reinterpret_cast<const char*>(written.data()) +
				label_offset + label_size,
			6) == "second" );
	}

	SECTION( "a record is padded with spaces to its full width" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);
		header.add_label("short");

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));

		const auto *first =
			reinterpret_cast<const char*>(written.data()) + label_offset;

		REQUIRE( std::string(first + 5, label_size - 5) ==
			std::string(label_size - 5, ' ') );
	}

	SECTION( "a file that uses no label states none" )
	{
		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));
		const auto reread = parse_header(view(written));

		std::int32_t stated = 0;
		std::memcpy(&stated, written.data() + nlabl_offset, sizeof(stated));

		REQUIRE( stated == 0 );
		REQUIRE( reread.get_labels().empty() );
	}

	SECTION( "a label of the full width survives" )
	{
		const std::string full(label_size, 'x');

		mrc_header header;
		header.set_mode(mrc_mode::float32);
		header.set_column_axis(1);
		header.set_row_axis(2);
		header.set_section_axis(3);
		header.add_label(full);

		raw_header written(header_size, rexlib::byte{});
		serialize_header(header, make_span(written.data(), written.size()));
		const auto reread = parse_header(view(written));

		REQUIRE( reread.get_labels()[0] == full );
	}
}

TEST_CASE( "a label an MRC file cannot carry is refused", "[mrc_header]" )
{
	mrc_header header;

	SECTION( "one longer than a record is refused" )
	{
		REQUIRE_THROWS_AS(
			header.add_label(std::string(label_size + 1, 'x')),
			std::invalid_argument
		);
	}

	SECTION( "one holding anything but printable ASCII is refused" )
	{
		// A record is fixed width text, so a terminator or a control
		// character in the middle of one would be read back as something
		// else.
		REQUIRE_THROWS_AS(
			header.add_label(std::string("a\0b", 3)),
			std::invalid_argument
		);
		REQUIRE_THROWS_AS( header.add_label("a\tb"), std::invalid_argument );
	}

	SECTION( "an eleventh label is refused" )
	{
		for (std::size_t i = 0; i < 10; ++i)
		{
			header.add_label("filler");
		}

		REQUIRE_THROWS_AS( header.add_label("one too many"),
			std::out_of_range );
	}
}
