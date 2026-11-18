// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/preprocessing.hpp>

#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/program_service.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_index.hpp>
#include <rexlib/core/hardware/device_service.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/functional/creation.hpp>

#include <cstddef>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace rexlib;

namespace
{

using extent_list_type = std::vector<std::size_t>;

/**
 * @brief An execution context on the CPU.
 */
class cpu_context
{
public:
	cpu_context()
	{
		const auto devices = m_catalog.get_service<device_service>();
		const auto programs = m_catalog.get_service<program_service>();
		const auto session =
			devices->create_device_session(device_index("cpu", 0));
		m_context = execution_context(
			device_context(session),
			make_eager_dispatcher(programs)
		);
	}

	const execution_context& get() const noexcept
	{
		return m_context;
	}

private:
	service_catalog m_catalog;
	execution_context m_context;
};

std::size_t count_of(const extent_list_type &extents)
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
	}
	return result;
}

/**
 * @brief An array of uniform random values in [-1, 1].
 */
template <typename T>
array make_random(
	const execution_context &context,
	const extent_list_type &extents,
	unsigned seed
)
{
	auto result = empty(
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type_of<T>::value
		),
		memory_resource_affinity::device,
		context
	);
	auto *data = static_cast<T*>(result.get_storage()->get_host_ptr());
	std::mt19937 engine(seed);
	std::uniform_real_distribution<double> unit(-1.0, 1.0);
	for (std::size_t i = 0; i < count_of(extents); ++i)
	{
		data[i] = static_cast<T>(unit(engine));
	}
	return result;
}

std::string describe(const extent_list_type &extents)
{
	std::ostringstream oss;
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		oss << (i ? "x" : " ") << extents[i];
	}
	return oss.str();
}

} // anonymous namespace

TEST_CASE(
	"background standardization of a stack of particles and of a micrograph",
	"[preprocessing]"
)
{
	const cpu_context context;
	const std::vector<extent_list_type> shapes = {
		{ 256, 128, 128 },
		{ 2048, 2048 }
	};
	for (const auto &shape : shapes)
	{
		const auto images = make_random<float>(context.get(), shape, 61);
		const auto key = zeros(
			array_descriptor(
				strided_layout::make_contiguous_layout(
					span<const std::size_t>()
				),
				numerical_type::uint64
			),
			memory_resource_affinity::device,
			context.get()
		);
		const auto radius = 0.4 * static_cast<double>(shape.back());
		auto result = em::standardize_background_2d(
			images,
			key,
			radius,
			-3,
			3,
			context.get()
		);

		BENCHMARK("standardize_background_2d" + describe(shape))
		{
			return em::standardize_background_2d(
				images,
				key,
				radius,
				-3,
				3,
				context.get(),
				&result
			);
		};
	}
}
