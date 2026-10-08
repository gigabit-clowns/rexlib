// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace rexlib
{
namespace test
{

/**
 * @brief Count the elements a set of extents holds.
 *
 * @param extents The extents.
 * @return std::size_t Their product.
 */
inline std::size_t element_count(const std::vector<std::size_t> &extents)
{
	return std::accumulate(
		extents.cbegin(),
		extents.cend(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

/**
 * @brief Make a contiguous array on the host, its elements zeroed.
 *
 * @tparam T Type of one element.
 * @param extents Extents of the array.
 * @param data_type The data type @p T stands for.
 * @return array The array.
 */
template <typename T>
array make_host_array(
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	const auto count = element_count(extents);
	auto storage = std::make_shared<host_buffer>(
		count * sizeof(T),
		alignof(T)
	);
	std::fill_n(static_cast<T*>(storage->get_host_ptr()), count, T(0));

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			data_type
		)
	);
}

/**
 * @brief Make a contiguous array on the host holding given values.
 *
 * @tparam T Type of one element.
 * @param extents Extents of the array.
 * @param data_type The data type @p T stands for.
 * @param values The values, as many as @p extents hold.
 * @return array The array.
 */
template <typename T>
array make_host_array(
	const std::vector<std::size_t> &extents,
	numerical_type data_type,
	const std::vector<T> &values
)
{
	auto result = make_host_array<T>(extents, data_type);
	std::copy(
		values.cbegin(),
		values.cend(),
		static_cast<T*>(result.get_storage()->get_host_ptr())
	);

	return result;
}

/**
 * @brief Get the values a contiguous array on the host holds.
 *
 * @tparam T Type of one element.
 * @param values The array.
 * @param extents Extents of the array.
 * @return std::vector<T> Its values, in order.
 */
template <typename T>
std::vector<T> values_of(
	const array &values,
	const std::vector<std::size_t> &extents
)
{
	const auto *data = static_cast<const T*>(
		values.get_storage()->get_host_ptr());

	return std::vector<T>(data, data + element_count(extents));
}

/**
 * @brief Make the plan of one region covering a whole file and a whole
 * array of the same extents.
 *
 * @param extents Extents of both.
 * @return image_transfer_plan The plan.
 */
inline em::image_transfer_plan
whole_of(const std::vector<std::size_t> &extents)
{
	em::image_transfer_plan regions(
		em::image_transfer_shape(extents, extents.size(), extents.size())
	);
	const std::vector<std::size_t> origin(extents.size(), 0);
	regions.add(make_span(origin), make_span(origin));

	return regions;
}

} // namespace test
} // namespace rexlib
