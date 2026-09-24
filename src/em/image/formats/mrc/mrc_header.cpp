// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_header.hpp"

#include "mrc_constants.hpp"

#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace
{

template <typename T>
T read_scalar(span<const byte> bytes, std::size_t offset, byte_order order)
{
	typename std::make_unsigned<
		typename std::conditional<
			std::is_integral<T>::value, T, std::uint32_t
		>::type
	>::type raw;
	static_assert(
		sizeof(raw) == sizeof(T),
		"The raw type must have the width of the field."
	);

	std::memcpy(&raw, bytes.data() + offset, sizeof(raw));
	raw = convert_byte_order(raw, order, get_system_byte_order());

	T value;
	std::memcpy(&value, &raw, sizeof(value));
	return value;
}

template <typename T>
void write_scalar(
	span<byte> bytes,
	std::size_t offset,
	T value,
	byte_order order
)
{
	typename std::make_unsigned<
		typename std::conditional<
			std::is_integral<T>::value, T, std::uint32_t
		>::type
	>::type raw;
	static_assert(
		sizeof(raw) == sizeof(T),
		"The raw type must have the width of the field."
	);

	std::memcpy(&raw, &value, sizeof(raw));
	raw = convert_byte_order(raw, get_system_byte_order(), order);
	std::memcpy(bytes.data() + offset, &raw, sizeof(raw));
}

mrc_header::vector3f_type
read_vector3f(span<const byte> bytes, std::size_t offset, byte_order order)
{
	mrc_header::vector3f_type value;
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		value[i] = read_scalar<float>(
			bytes, offset + i * sizeof(float), order
		);
	}

	return value;
}

void write_vector3f(
	span<byte> bytes,
	std::size_t offset,
	const mrc_header::vector3f_type &value,
	byte_order order
)
{
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		write_scalar(bytes, offset + i * sizeof(float), value[i], order);
	}
}

bool is_label_padding(char c) noexcept
{
	return c == ' ' || c == '\0';
}

std::string read_label(span<const byte> bytes, std::size_t index)
{
	const auto *first = reinterpret_cast<const char*>(bytes.data()) +
		offset::label + index * size::label;

	auto length = size::label;
	while (length > 0 && is_label_padding(first[length-1]))
	{
		--length;
	}

	return std::string(first, length);
}

void write_label(span<byte> bytes, std::size_t index, const std::string &text)
{
	REXLIB_ASSERT(text.size() <= size::label);
	auto *first = reinterpret_cast<char*>(bytes.data()) +
		offset::label + index * size::label;

	std::memcpy(first, text.data(), text.size());
	std::memset(
		first + text.size(),
		' ',
		size::label - text.size()
	);
}

bool is_printable_ascii(const std::string &text) noexcept
{
	return std::all_of(
		text.cbegin(),
		text.cend(),
		[] (char character)
		{
			const auto value = static_cast<unsigned char>(character);
			return value >= 0x20 && value < 0x7F;
		}
	);
}

bool matches_stamp(
	span<const byte> bytes,
	const std::array<std::uint8_t, 2> &stamp
)
{
	return as_uint8(bytes[offset::machst]) == stamp[0] &&
		as_uint8(bytes[offset::machst + 1]) == stamp[1];
}

bool read_machine_stamp(span<const byte> bytes, byte_order &order)
{
	if (matches_stamp(bytes, little_endian_machine_stamp) ||
		matches_stamp(bytes, legacy_little_endian_machine_stamp))
	{
		order = byte_order::little_endian;
		return true;
	}

	if (matches_stamp(bytes, big_endian_machine_stamp))
	{
		order = byte_order::big_endian;
		return true;
	}

	return false;
}

bool resolves_mode(span<const byte> bytes, byte_order order)
{
	const auto mode = static_cast<mrc_mode>(
		read_scalar<std::int32_t>(bytes, offset::mode, order)
	);

	return is_supported(mode);
}

// The stamp names the byte order and the mode confirms it. A file whose data
// was byte swapped without its stamp being updated is resolved by the mode
// alone, and one whose mode resolves neither way is not readable at all.
byte_order resolve_byte_order(span<const byte> bytes)
{
	byte_order order;
	if (read_machine_stamp(bytes, order) && resolves_mode(bytes, order))
	{
		return order;
	}

	if (resolves_mode(bytes, byte_order::little_endian))
	{
		return byte_order::little_endian;
	}

	if (resolves_mode(bytes, byte_order::big_endian))
	{
		return byte_order::big_endian;
	}

	throw image_format_error(
		"mrc::parse_header: No byte order makes the mode of the file one "
		"this format supports."
	);
}

void validate(const mrc_header &header)
{
	if (header.get_column_count() < 0 ||
		header.get_row_count() < 0 ||
		header.get_section_count() < 0 ||
		header.get_extended_header_size() < 0)
	{
		throw image_format_error(
			"mrc::parse_header: The file states a negative extent or "
			"extended header size."
		);
	}

	if (!has_axis_permutation(header) && !has_unset_axes(header))
	{
		throw image_format_error(
			"mrc::parse_header: The axis correspondence of the file names "
			"anything but the three axes of space, one each."
		);
	}

	if (is_volume_stack_space_group(header.get_space_group()))
	{
		const auto sampling = header.get_section_sampling();
		if (sampling <= 0 || header.get_section_count() % sampling != 0)
		{
			throw image_format_error(
				"mrc::parse_header: The file states a stack of volumes "
				"whose section count is not a multiple of its volume "
				"depth."
			);
		}
	}
}

} // anonymous namespace

mrc_header::mrc_header() noexcept
	: m_byte_order(get_system_byte_order())
	, m_column_count(0)
	, m_row_count(0)
	, m_section_count(0)
	, m_mode(mrc_mode::int8)
	, m_column_sampling(0)
	, m_row_sampling(0)
	, m_section_sampling(0)
	, m_cell_size{{0.0F, 0.0F, 0.0F}}
	, m_cell_angles{{0.0F, 0.0F, 0.0F}}
	, m_column_axis(0)
	, m_row_axis(0)
	, m_section_axis(0)
	, m_data_min(0.0F)
	, m_data_max(0.0F)
	, m_data_mean(0.0F)
	, m_data_rms(0.0F)
	, m_space_group(0)
	, m_extended_header_size(0)
	, m_version(0)
	, m_imod_stamp(0)
	, m_imod_flags(0)
{
}

byte_order mrc_header::get_byte_order() const noexcept
{
	return m_byte_order;
}

void mrc_header::set_byte_order(byte_order order) noexcept
{
	m_byte_order = order;
}

std::int32_t mrc_header::get_column_count() const noexcept
{
	return m_column_count;
}

void mrc_header::set_column_count(std::int32_t count) noexcept
{
	m_column_count = count;
}

std::int32_t mrc_header::get_row_count() const noexcept
{
	return m_row_count;
}

void mrc_header::set_row_count(std::int32_t count) noexcept
{
	m_row_count = count;
}

std::int32_t mrc_header::get_section_count() const noexcept
{
	return m_section_count;
}

void mrc_header::set_section_count(std::int32_t count) noexcept
{
	m_section_count = count;
}

mrc_mode mrc_header::get_mode() const noexcept
{
	return m_mode;
}

void mrc_header::set_mode(mrc_mode mode) noexcept
{
	m_mode = mode;
}

std::int32_t mrc_header::get_column_sampling() const noexcept
{
	return m_column_sampling;
}

void mrc_header::set_column_sampling(std::int32_t sampling) noexcept
{
	m_column_sampling = sampling;
}

std::int32_t mrc_header::get_row_sampling() const noexcept
{
	return m_row_sampling;
}

void mrc_header::set_row_sampling(std::int32_t sampling) noexcept
{
	m_row_sampling = sampling;
}

std::int32_t mrc_header::get_section_sampling() const noexcept
{
	return m_section_sampling;
}

void mrc_header::set_section_sampling(std::int32_t sampling) noexcept
{
	m_section_sampling = sampling;
}

const mrc_header::vector3f_type& mrc_header::get_cell_size() const noexcept
{
	return m_cell_size;
}

void mrc_header::set_cell_size(const vector3f_type &size) noexcept
{
	m_cell_size = size;
}

const mrc_header::vector3f_type& mrc_header::get_cell_angles() const noexcept
{
	return m_cell_angles;
}

void mrc_header::set_cell_angles(const vector3f_type &angles) noexcept
{
	m_cell_angles = angles;
}

std::int32_t mrc_header::get_column_axis() const noexcept
{
	return m_column_axis;
}

void mrc_header::set_column_axis(std::int32_t axis) noexcept
{
	m_column_axis = axis;
}

std::int32_t mrc_header::get_row_axis() const noexcept
{
	return m_row_axis;
}

void mrc_header::set_row_axis(std::int32_t axis) noexcept
{
	m_row_axis = axis;
}

std::int32_t mrc_header::get_section_axis() const noexcept
{
	return m_section_axis;
}

void mrc_header::set_section_axis(std::int32_t axis) noexcept
{
	m_section_axis = axis;
}

float mrc_header::get_data_min() const noexcept
{
	return m_data_min;
}

void mrc_header::set_data_min(float value) noexcept
{
	m_data_min = value;
}

float mrc_header::get_data_max() const noexcept
{
	return m_data_max;
}

void mrc_header::set_data_max(float value) noexcept
{
	m_data_max = value;
}

float mrc_header::get_data_mean() const noexcept
{
	return m_data_mean;
}

void mrc_header::set_data_mean(float value) noexcept
{
	m_data_mean = value;
}

float mrc_header::get_data_rms() const noexcept
{
	return m_data_rms;
}

void mrc_header::set_data_rms(float value) noexcept
{
	m_data_rms = value;
}

std::int32_t mrc_header::get_space_group() const noexcept
{
	return m_space_group;
}

void mrc_header::set_space_group(std::int32_t space_group) noexcept
{
	m_space_group = space_group;
}

std::int32_t mrc_header::get_extended_header_size() const noexcept
{
	return m_extended_header_size;
}

void mrc_header::set_extended_header_size(std::int32_t size) noexcept
{
	m_extended_header_size = size;
}

std::int32_t mrc_header::get_version() const noexcept
{
	return m_version;
}

void mrc_header::set_version(std::int32_t version) noexcept
{
	m_version = version;
}

std::int32_t mrc_header::get_imod_stamp() const noexcept
{
	return m_imod_stamp;
}

void mrc_header::set_imod_stamp(std::int32_t stamp) noexcept
{
	m_imod_stamp = stamp;
}

std::int32_t mrc_header::get_imod_flags() const noexcept
{
	return m_imod_flags;
}

void mrc_header::set_imod_flags(std::int32_t flags) noexcept
{
	m_imod_flags = flags;
}

span<const std::string> mrc_header::get_labels() const noexcept
{
	return make_span(m_labels.data(), m_labels.size());
}

void mrc_header::add_label(const std::string &label)
{
	if (m_labels.size() >= label_count)
	{
		throw std::out_of_range(
			"mrc_header::add_label: An MRC file holds no more labels."
		);
	}

	if (label.size() > size::label)
	{
		throw std::invalid_argument(
			"mrc_header::add_label: The label is longer than a label record."
		);
	}

	if (!is_printable_ascii(label))
	{
		throw std::invalid_argument(
			"mrc_header::add_label: A label holds printable ASCII only."
		);
	}

	m_labels.push_back(label);
}

bool has_map_identifier(span<const byte> bytes) noexcept
{
	if (bytes.size() < offset::map + map_id_match_size)
	{
		return false;
	}

	for (std::size_t i = 0; i < map_id_match_size; ++i)
	{
		if (as_uint8(bytes[offset::map + i]) !=
			static_cast<std::uint8_t>(map_id[i]))
		{
			return false;
		}
	}

	return true;
}

mrc_header parse_header(span<const byte> bytes)
{
	if (bytes.size() < header_size)
	{
		throw image_format_error(
			"mrc::parse_header: The file is shorter than an MRC header."
		);
	}

	if (!has_map_identifier(bytes))
	{
		throw image_format_error(
			"mrc::parse_header: The file does not carry the MRC "
			"identifier."
		);
	}

	const auto order = resolve_byte_order(bytes);

	mrc_header header;
	header.set_byte_order(order);
	header.set_column_count(
		read_scalar<std::int32_t>(bytes, offset::nx, order));
	header.set_row_count(
		read_scalar<std::int32_t>(bytes, offset::ny, order));
	header.set_section_count(
		read_scalar<std::int32_t>(bytes, offset::nz, order));
	header.set_mode(static_cast<mrc_mode>(
		read_scalar<std::int32_t>(bytes, offset::mode, order)));
	header.set_column_sampling(
		read_scalar<std::int32_t>(bytes, offset::mx, order));
	header.set_row_sampling(
		read_scalar<std::int32_t>(bytes, offset::my, order));
	header.set_section_sampling(
		read_scalar<std::int32_t>(bytes, offset::mz, order));
	header.set_cell_size(read_vector3f(bytes, offset::cella, order));
	header.set_cell_angles(read_vector3f(bytes, offset::cellb, order));
	header.set_column_axis(
		read_scalar<std::int32_t>(bytes, offset::mapc, order));
	header.set_row_axis(
		read_scalar<std::int32_t>(bytes, offset::mapr, order));
	header.set_section_axis(
		read_scalar<std::int32_t>(bytes, offset::maps, order));
	header.set_data_min(read_scalar<float>(bytes, offset::dmin, order));
	header.set_data_max(read_scalar<float>(bytes, offset::dmax, order));
	header.set_data_mean(read_scalar<float>(bytes, offset::dmean, order));
	header.set_data_rms(read_scalar<float>(bytes, offset::rms, order));
	header.set_space_group(
		read_scalar<std::int32_t>(bytes, offset::ispg, order));
	header.set_extended_header_size(
		read_scalar<std::int32_t>(bytes, offset::nsymbt, order));
	header.set_version(
		read_scalar<std::int32_t>(bytes, offset::nversion, order));
	header.set_imod_stamp(
		read_scalar<std::int32_t>(bytes, offset::imod_stamp, order));
	header.set_imod_flags(
		read_scalar<std::int32_t>(bytes, offset::imod_flags, order));

	// Only the labels the file says it uses are taken. A count past the room
	// there is says nothing trustworthy about the rest, so it is clamped
	// rather than treated as a reason to refuse the file.
	const auto stated = read_scalar<std::int32_t>(bytes, offset::nlabl, order);
	const auto used = std::min(
		static_cast<std::size_t>(std::max(stated, 0)),
		label_count
	);
	for (std::size_t i = 0; i < used; ++i)
	{
		const auto text = read_label(bytes, i);
		if (is_printable_ascii(text))
		{
			header.add_label(text);
		}
	}

	validate(header);

	return header;
}

void serialize_header(const mrc_header &header, span<byte> bytes)
{
	if (bytes.size() < header_size)
	{
		throw std::invalid_argument(
			"mrc::serialize_header: The destination is shorter than an MRC "
			"header."
		);
	}

	std::memset(bytes.data(), 0, header_size);

	const auto order = header.get_byte_order();
	write_scalar(bytes, offset::nx, header.get_column_count(), order);
	write_scalar(bytes, offset::ny, header.get_row_count(), order);
	write_scalar(bytes, offset::nz, header.get_section_count(), order);
	write_scalar(
		bytes,
		offset::mode,
		static_cast<std::int32_t>(header.get_mode()),
		order
	);
	write_scalar(bytes, offset::mx, header.get_column_sampling(), order);
	write_scalar(bytes, offset::my, header.get_row_sampling(), order);
	write_scalar(bytes, offset::mz, header.get_section_sampling(), order);
	write_vector3f(bytes, offset::cella, header.get_cell_size(), order);
	write_vector3f(bytes, offset::cellb, header.get_cell_angles(), order);
	write_scalar(bytes, offset::mapc, header.get_column_axis(), order);
	write_scalar(bytes, offset::mapr, header.get_row_axis(), order);
	write_scalar(bytes, offset::maps, header.get_section_axis(), order);
	write_scalar(bytes, offset::dmin, header.get_data_min(), order);
	write_scalar(bytes, offset::dmax, header.get_data_max(), order);
	write_scalar(bytes, offset::dmean, header.get_data_mean(), order);
	write_scalar(bytes, offset::rms, header.get_data_rms(), order);
	write_scalar(bytes, offset::ispg, header.get_space_group(), order);
	write_scalar(
		bytes, offset::nsymbt, header.get_extended_header_size(), order);
	write_scalar(bytes, offset::nversion, header.get_version(), order);
	write_scalar(bytes, offset::imod_stamp, header.get_imod_stamp(), order);
	write_scalar(bytes, offset::imod_flags, header.get_imod_flags(), order);

	std::memcpy(bytes.data() + offset::map, map_id, size::map);

	const auto &labels = header.get_labels();
	write_scalar(
		bytes,
		offset::nlabl,
		static_cast<std::int32_t>(labels.size()),
		order
	);
	for (std::size_t i = 0; i < labels.size(); ++i)
	{
		write_label(bytes, i, labels[i]);
	}

	const auto &stamp = order == byte_order::little_endian
		? little_endian_machine_stamp
		: big_endian_machine_stamp;
	bytes[offset::machst] = as_byte(stamp[0]);
	bytes[offset::machst + 1] = as_byte(stamp[1]);
}

bool has_axis_permutation(const mrc_header &header) noexcept
{
	std::array<std::int32_t, space_axis_count> axes = {{
		header.get_column_axis(),
		header.get_row_axis(),
		header.get_section_axis()
	}};
	std::sort(axes.begin(), axes.end());

	return axes[0] == 1 && axes[1] == 2 && axes[2] == 3;
}

bool has_unset_axes(const mrc_header &header) noexcept
{
	return header.get_column_axis() == 0 &&
		header.get_row_axis() == 0 &&
		header.get_section_axis() == 0;
}

bool holds_signed_bytes(const mrc_header &header) noexcept
{
	if (header.get_imod_stamp() != imod_stamp_value)
	{
		return true;
	}

	return (header.get_imod_flags() & imod_signed_bytes_flag) != 0;
}

numerical_type get_data_type(const mrc_header &header) noexcept
{
	return get_data_type(header.get_mode(), holds_signed_bytes(header));
}

std::size_t get_data_offset(const mrc_header &header) noexcept
{
	return header_size +
		static_cast<std::size_t>(header.get_extended_header_size());
}

} // namespace mrc
} // namespace em
} // namespace rexlib
