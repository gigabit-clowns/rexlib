// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/batch_reduce_map_program_builder.hpp>
#include <backends/cpu/hardware/cpu_command_queue.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>
#include <backends/cpu/loops/strided_pointer_iterator.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <rexlib/backends/cpu/cpu_program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>
#include <rexlib/ops/fourier/fftshift_operation.hpp>

#include "../serial_pool.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

template <typename... Types>
struct float64_only : std::is_same<std::tuple<Types...>, std::tuple<double>>
{
};

/**
 * @brief Sums the elements it folds, and finalizes to their mean.
 *
 * With a threshold, it folds only the elements at or above it.
 */
class mean_kernel
{
public:
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<double, std::size_t>;
	};

	explicit mean_kernel(double threshold) noexcept
		: m_threshold(threshold)
	{
	}

	void seed(double &total, std::size_t &count, const double *value) const
	{
		const auto kept = *value >= m_threshold;
		total = kept ? *value : 0.0;
		count = kept ? 1 : 0;
	}

	void combine(double &total, std::size_t &count, const double *value) const
	{
		if (*value >= m_threshold)
		{
			total += *value;
			++count;
		}
	}

	void merge(
		double &total,
		std::size_t &count,
		const double &other_total,
		const std::size_t &other_count
	) const
	{
		total += other_total;
		count += other_count;
	}

	void finalize(
		double *mean,
		const double &total,
		const std::size_t &count,
		std::size_t
	) const
	{
		*mean = count ? total / static_cast<double>(count) : 0.0;
	}

private:
	double m_threshold;
};

/**
 * @brief Writes every element less a value.
 */
class subtract_kernel
{
public:
	explicit subtract_kernel(double value) noexcept
		: m_value(value)
	{
	}

	template <typename Pointers, typename Strides>
	void operator()(
		const Pointers &pointers,
		const Strides &strides,
		std::size_t count
	) const
	{
		auto result = make_strided_pointer_iterator(
			std::get<0>(pointers),
			std::get<0>(strides)
		);
		auto value = make_strided_pointer_iterator(
			std::get<1>(pointers),
			std::get<1>(strides)
		);
		for (std::size_t i = 0; i < count; ++i)
		{
			result[static_cast<std::ptrdiff_t>(i)] =
				value[static_cast<std::ptrdiff_t>(i)] - m_value;
		}
	}

private:
	double m_value;
};

/**
 * @brief Subtracts from every image the mean of its elements, or with two
 * passes the mean of its elements at or above that mean.
 */
template <std::size_t PassCount>
class mean_subtraction
{
public:
	mean_kernel make_reduction(
		std::size_t pass,
		const std::array<double, 1> &previous
	) const
	{
		return mean_kernel(pass == 0 ? -1e300 : previous[0]);
	}

	template <typename Inputs>
	subtract_kernel make_map(
		const std::array<double, 1> &statistics,
		const Inputs&
	) const
	{
		return subtract_kernel(statistics[0]);
	}
};

template <std::size_t PassCount>
struct mean_subtraction_factory
{
	static REXLIB_CONST_CONSTEXPR std::size_t statistic_count = 1;
	static REXLIB_CONST_CONSTEXPR std::size_t pass_count = PassCount;

	template <typename... Outs, typename... Ins>
	mean_subtraction<PassCount> operator()(
		const ops::fftshift_operation&,
		type_list<Outs...>,
		type_list<Ins...>,
		const batch_reduce_map_plan&
	) const
	{
		return mean_subtraction<PassCount>();
	}
};

template <std::size_t PassCount>
using mean_subtraction_builder = batch_reduce_map_program_builder<
	ops::fftshift_operation,
	mean_subtraction_factory<PassCount>,
	rule_type_dispatcher<
		ops::fftshift_operation::type_rule_type,
		float64_only
	>
>;

/**
 * @brief Counts the elements whose value is not 10 times their row plus
 * their column.
 */
class index_check_kernel
{
public:
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<double>;
	};

	void seed(
		double &mismatches,
		const double *value,
		const multidimensional_index &index
	) const
	{
		mismatches = mismatch(*value, index);
	}

	void combine(
		double &mismatches,
		const double *value,
		const multidimensional_index &index
	) const
	{
		mismatches += mismatch(*value, index);
	}

	void merge(double &mismatches, const double &other) const
	{
		mismatches += other;
	}

	void finalize(double *result, const double &mismatches, std::size_t) const
	{
		*result = mismatches;
	}

private:
	static double mismatch(double value, const multidimensional_index &index)
	{
		const auto expected = static_cast<double>(10*index[0] + index[1]);
		return value == expected ? 0.0 : 1.0;
	}
};

/**
 * @brief Writes the counter of every element, plus 1000 times its seed and
 * a million times the mismatches of its image.
 */
class counter_kernel
{
public:
	explicit counter_kernel(double base) noexcept
		: m_base(base)
	{
	}

	template <typename Pointers, typename Strides, typename IndexRun>
	void operator()(
		const Pointers &pointers,
		const Strides &strides,
		std::size_t count,
		const IndexRun &index
	) const
	{
		auto result = make_strided_pointer_iterator(
			std::get<0>(pointers),
			std::get<0>(strides)
		);
		for (std::size_t i = 0; i < count; ++i)
		{
			result[static_cast<std::ptrdiff_t>(i)] = m_base +
				static_cast<double>(index.advanced(i).get_index());
		}
	}

private:
	double m_base;
};

class counter_writer
{
public:
	index_check_kernel make_reduction(
		std::size_t,
		const std::array<double, 1>&
	) const
	{
		return index_check_kernel();
	}

	template <typename Inputs>
	counter_kernel make_map(
		const std::array<double, 1> &statistics,
		const Inputs &inputs,
		std::size_t counter
	) const
	{
		return counter_kernel(
			1e6 * statistics[0] +
			1000.0 * static_cast<double>(*std::get<1>(inputs)) +
			static_cast<double>(counter)
		);
	}
};

struct counter_writer_factory
{
	static REXLIB_CONST_CONSTEXPR std::size_t statistic_count = 1;
	static REXLIB_CONST_CONSTEXPR std::size_t pass_count = 1;

	template <typename... Outs, typename... Ins>
	counter_writer operator()(
		const em::standardize_background_operation&,
		type_list<Outs...>,
		type_list<Ins...>,
		const batch_reduce_map_plan&
	) const
	{
		return counter_writer();
	}
};

using counter_writer_builder = batch_reduce_map_program_builder<
	em::standardize_background_operation,
	counter_writer_factory,
	rule_type_dispatcher<
		em::standardize_background_operation::type_rule_type,
		float64_only
	>,
	multidimensional_index_tag,
	linear_index_tag
>;

operand_descriptor make_operand_descriptor(
	std::vector<std::size_t> extents,
	numerical_type data_type = numerical_type::float64
)
{
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type,
		nullptr
	);
}

std::size_t count_elements(const std::vector<std::size_t> &extents)
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
	}
	return result;
}

template <typename T>
std::shared_ptr<host_buffer> make_buffer(const std::vector<T> &values)
{
	auto result = std::make_shared<host_buffer>(
		(values.empty() ? 1 : values.size()) * sizeof(T),
		alignof(T)
	);
	auto *data = static_cast<T*>(result->get_host_ptr());
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		data[i] = values[i];
	}
	return result;
}

template <typename T>
std::vector<T> read_buffer(const host_buffer &buffer, std::size_t count)
{
	const auto *data = static_cast<const T*>(buffer.get_host_ptr());
	return std::vector<T>(data, data + count);
}

/**
 * @brief Build and run a program over one output and the given inputs.
 */
template <typename Builder, typename Op>
std::vector<double> run(
	const Op &operation,
	const std::vector<std::size_t> &extents,
	const std::vector<operand_descriptor> &input_descriptors,
	const std::vector<std::shared_ptr<host_buffer>> &input_buffers,
	thread_pool &pool = *get_serial_pool()
)
{
	const Builder builder;
	cpu_command_queue queue(get_serial_pool());
	const std::vector<operand_descriptor> output_descriptors = {
		make_operand_descriptor(extents)
	};

	const auto program = builder.build(
		operation,
		make_span(output_descriptors),
		make_span(input_descriptors),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto count = count_elements(extents);
	const auto output = make_buffer(std::vector<double>(count, -1.0));
	const std::vector<std::shared_ptr<buffer>> outputs = { output };
	const std::vector<std::shared_ptr<const buffer>> inputs(
		input_buffers.begin(),
		input_buffers.end()
	);
	const std::vector<std::shared_ptr<buffer>> scratches;

	dynamic_cast<cpu_program&>(*program).execute(
		make_span(outputs),
		make_span(inputs),
		make_span(scratches),
		pool
	);
	return read_buffer<double>(*output, count);
}

template <std::size_t PassCount>
std::vector<double> run_mean_subtraction(
	const std::vector<std::size_t> &extents,
	const ops::axis_list &axes,
	const std::vector<double> &values,
	thread_pool &pool = *get_serial_pool()
)
{
	return run<mean_subtraction_builder<PassCount>>(
		ops::fftshift_operation(axes),
		extents,
		{ make_operand_descriptor(extents) },
		{ make_buffer(values) },
		pool
	);
}

std::vector<double> make_ramp(std::size_t count)
{
	std::vector<double> result(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		result[i] = static_cast<double>((i * 7) % 23);
	}
	return result;
}

} // anonymous namespace

TEST_CASE(
	"batch_reduce_map_program_builder should map every image with the "
	"statistics of its own core",
	"[batch_reduce_map_program_builder]"
)
{
	// Two images of 2 x 2 along axes 0 and 2, batched along axis 1: image 0
	// holds 1, 3, 5 and 7, and image 1 holds 10, 30, 50 and 70.
	const std::vector<double> values = { 1, 3, 10, 30, 5, 7, 50, 70 };

	const auto result = run_mean_subtraction<1>({ 2, 2, 2 }, { 0, 2 }, values);

	CHECK(
		result ==
		std::vector<double>{ -3, -1, -30, -10, 1, 3, 10, 30 }
	);
}

TEST_CASE(
	"batch_reduce_map_program_builder should hand every reduction the "
	"statistics of the one before",
	"[batch_reduce_map_program_builder]"
)
{
	// The mean of 0, 0, 0 and 8 is 2, and the mean of what is at or above
	// it is 8.
	const std::vector<double> values = { 0, 0, 0, 8 };

	CHECK(
		run_mean_subtraction<2>({ 4 }, { 0 }, values) ==
		std::vector<double>{ -8, -8, -8, 0 }
	);
}

TEST_CASE(
	"batch_reduce_map_program_builder should hand the map the inputs without "
	"core axes and the counter",
	"[batch_reduce_map_program_builder]"
)
{
	// Three images of 2 x 3, every element being 10 times its row plus its
	// column, so that the reduction finds no mismatch.
	const std::vector<std::size_t> extents = { 3, 2, 3 };
	std::vector<double> values;
	for (std::size_t image = 0; image < 3; ++image)
	{
		for (std::size_t element = 0; element < 6; ++element)
		{
			values.push_back(static_cast<double>(10*(element / 3) +
				element % 3));
		}
	}

	SECTION( "one seed per image" )
	{
		const auto result = run<counter_writer_builder>(
			em::standardize_background_operation({ 1, 2 }, 1, -1, 1),
			extents,
			{
				make_operand_descriptor(extents),
				make_operand_descriptor({ 3 }, numerical_type::uint64)
			},
			{
				make_buffer(values),
				make_buffer(std::vector<std::uint64_t>{ 4, 5, 6 })
			}
		);

		for (std::size_t i = 0; i < result.size(); ++i)
		{
			INFO( "element " << i );
			CHECK(
				result[i] ==
				1000.0 * static_cast<double>(4 + i / 6) +
				static_cast<double>(i % 6)
			);
		}
	}

	SECTION( "one seed for every image" )
	{
		const auto result = run<counter_writer_builder>(
			em::standardize_background_operation({ 1, 2 }, 1, -1, 1),
			extents,
			{
				make_operand_descriptor(extents),
				make_operand_descriptor({}, numerical_type::uint64)
			},
			{
				make_buffer(values),
				make_buffer(std::vector<std::uint64_t>{ 9 })
			}
		);

		for (std::size_t i = 0; i < result.size(); ++i)
		{
			INFO( "element " << i );
			CHECK( result[i] == 9000.0 + static_cast<double>(i) );
		}
	}
}

TEST_CASE(
	"batch_reduce_map_program_builder should compute what one thread "
	"computes, whether it shares out the images or each image",
	"[batch_reduce_map_program_builder]"
)
{
	thread_pool pool(3);

	SECTION( "many small images" )
	{
		const std::vector<std::size_t> extents = { 64, 32, 32 };
		const auto values = make_ramp(count_elements(extents));

		CHECK(
			run_mean_subtraction<2>(extents, { 1, 2 }, values, pool) ==
			run_mean_subtraction<2>(extents, { 1, 2 }, values)
		);
	}

	SECTION( "one large image" )
	{
		const std::vector<std::size_t> extents = { 1, 256, 256 };
		const auto values = make_ramp(count_elements(extents));

		CHECK(
			run_mean_subtraction<2>(extents, { 1, 2 }, values, pool) ==
			run_mean_subtraction<2>(extents, { 1, 2 }, values)
		);
	}
}

TEST_CASE(
	"batch_reduce_map_program_builder should run a single image without "
	"batch axes",
	"[batch_reduce_map_program_builder]"
)
{
	CHECK(
		run_mean_subtraction<1>({ 2, 2 }, { 0, 1 }, { 1, 2, 3, 6 }) ==
		std::vector<double>{ -2, -1, 0, 3 }
	);
}

TEST_CASE(
	"batch_reduce_map_program_builder should do nothing over an empty batch "
	"or an empty core",
	"[batch_reduce_map_program_builder]"
)
{
	CHECK( run_mean_subtraction<1>({ 0, 4 }, { 1 }, {}).empty() );
	CHECK( run_mean_subtraction<1>({ 3, 0 }, { 1 }, {}).empty() );
}
