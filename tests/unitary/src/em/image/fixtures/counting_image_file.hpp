// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <em/image/formats/memory_mapping/image_file_layout.hpp>
#include <em/image/formats/memory_mapping/mapped_image_reader.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace rexlib
{
namespace test
{

/**
 * @brief Get the number of elements of an array.
 *
 * @param extents Extents of the array.
 * @return std::size_t The product of the extents.
 */
inline std::size_t count_elements(const std::vector<std::size_t> &extents)
{
	return std::accumulate(
		extents.cbegin(),
		extents.cend(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

/**
 * @brief Make the float32 values first, first + 1, first + 2 and onwards.
 *
 * @param first The first value.
 * @param count Number of values.
 * @return std::vector<float> The values.
 */
inline std::vector<float> count_from(std::size_t first, std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(first + i);
	}

	return values;
}

/**
 * @brief Write a headerless file of float32 values that count up from zero.
 *
 * The values are in the byte order of the host. The value of an element is
 * its linear index, so an image file of extents (N, H, W) holds the value
 * (n * H + h) * W + w at (n, h, w).
 *
 * @param path Path to the file. It is replaced if it exists.
 * @param extents Extents of the image the file holds.
 */
inline void write_counting_image_file(
	const std::string &path,
	const std::vector<std::size_t> &extents
)
{
	const auto values = count_from(0, count_elements(extents));

	std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
	output.write(
		reinterpret_cast<const char*>(values.data()),
		static_cast<std::streamsize>(values.size() * sizeof(float))
	);
}

/**
 * @brief Open a file written by @ref write_counting_image_file.
 *
 * @param path Path to the file.
 * @param extents Extents of the image the file holds.
 * @param core_rank Number of trailing extents that are one image.
 * @return std::shared_ptr<const em::image_reader> A reader over the file.
 */
inline std::shared_ptr<const em::image_reader> open_counting_image_file(
	const std::string &path,
	const std::vector<std::size_t> &extents,
	std::size_t core_rank
)
{
	std::vector<std::ptrdiff_t> strides(extents.size());
	std::ptrdiff_t stride = 1;
	for (auto axis = extents.size(); axis > 0; --axis)
	{
		strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
	}

	return std::make_shared<em::mapped_image_reader>(
		path,
		em::image_file_layout(
			em::image_descriptor(
				make_span(extents),
				core_rank,
				numerical_type::float32
			),
			std::move(strides),
			0,
			get_system_byte_order()
		),
		em::image_metadata()
	);
}

/**
 * @brief Allocate a contiguous host array with every element set to a
 * value.
 *
 * @tparam T Element type. Must match @p data_type.
 * @param extents Extents of the array.
 * @param data_type Data type of the array.
 * @param value The value of every element.
 * @return array The array.
 */
template <typename T>
array make_host_array(
	const std::vector<std::size_t> &extents,
	numerical_type data_type,
	T value
)
{
	const auto count = count_elements(extents);
	auto storage = std::make_shared<host_buffer>(count * sizeof(T), alignof(T));
	std::fill_n(static_cast<T*>(storage->get_host_ptr()), count, value);

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			data_type
		)
	);
}

/**
 * @brief Copy the elements of a contiguous host array.
 *
 * @tparam T Element type. Must match the data type of @p values.
 * @param values The array.
 * @return std::vector<T> Its elements, in the order they are stored.
 */
template <typename T>
std::vector<T> get_values(const array &values)
{
	const auto *data = static_cast<const T*>(
		values.get_storage()->get_host_ptr()
	);
	const auto count =
		values.get_descriptor().get_layout().compute_element_count();

	return std::vector<T>(data, data + count);
}

} // namespace test
} // namespace rexlib
