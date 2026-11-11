// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/optics.hpp>

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

std::string describe(const extent_list_type &extents)
{
	std::ostringstream oss;
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		oss << (i ? "x" : " ") << extents[i];
	}
	return oss.str();
}

/**
 * @brief CTF parameters for a number of images, each with its own defocus.
 */
template <typename T>
array make_ctf_parameters(
	const execution_context &context,
	std::size_t count,
	std::size_t parameter_count
)
{
	auto result = empty(
		array_descriptor(
			strided_layout::make_contiguous_layout(
				make_span(extent_list_type{ count, parameter_count })
			),
			numerical_type_of<T>::value
		),
		memory_resource_affinity::device,
		context
	);
	auto *data = static_cast<T*>(result.get_storage()->get_host_ptr());
	std::mt19937 engine(81);
	std::uniform_real_distribution<double> defocus(5000.0, 30000.0);
	for (std::size_t i = 0; i < count * parameter_count; ++i)
	{
		data[i] = static_cast<T>(
			i % parameter_count < 2 ? defocus(engine) : 0.1
		);
	}
	return result;
}

} // anonymous namespace

TEST_CASE(
	"CTF of a stack of particles, of a micrograph and of many 1D profiles",
	"[optics]"
)
{
	const cpu_context context;
	const em::ctf_optics optics(300, 2.7, 0.1);

	for (const auto &shape : std::vector<extent_list_type>{
		{ 4096, 128 },
		{ 1, 4096 }
	})
	{
		const auto parameters =
			make_ctf_parameters<float>(context.get(), shape[0], 4);
		auto result = em::ctf_2d(
			parameters,
			shape[1],
			optics,
			1.0,
			context.get()
		);

		BENCHMARK("ctf_2d" + describe(shape))
		{
			return em::ctf_2d(
				parameters,
				shape[1],
				optics,
				1.0,
				context.get(),
				&result
			);
		};
	}

	const auto parameters =
		make_ctf_parameters<float>(context.get(), 100000, 2);
	auto result = em::ctf_1d(parameters, 64, optics, 1.0, context.get());
	BENCHMARK("ctf_1d 100000x64")
	{
		return em::ctf_1d(
			parameters,
			64,
			optics,
			1.0,
			context.get(),
			&result
		);
	};
}
