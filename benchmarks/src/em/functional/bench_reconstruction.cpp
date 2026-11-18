// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/reconstruction.hpp>

#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/program_service.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_index.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/device_service.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/functional/creation.hpp>

#include <array>
#include <cmath>
#include <complex>
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

template <typename T>
array make_array(
	const execution_context &context,
	const extent_list_type &extents,
	const std::vector<T> &values
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
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		data[i] = values[i];
	}
	return result;
}

array make_zeros(
	const execution_context &context,
	const extent_list_type &extents,
	numerical_type data_type
)
{
	return zeros(
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			data_type
		),
		memory_resource_affinity::device,
		context
	);
}

/**
 * @brief The operands of an insertion in float32: random slices under
 * uniformly drawn rotations, unit radial weights, and the accumulators.
 */
class insertion_operands
{
public:
	insertion_operands(
		const execution_context &context,
		std::size_t slice_extent,
		std::size_t volume_extent,
		std::size_t batch
	)
		: m_description(describe(slice_extent, volume_extent, batch))
	{
		const auto n = slice_extent;
		const auto h = n / 2 + 1;
		const extent_list_type volume = {
			volume_extent, volume_extent, volume_extent / 2 + 1
		};

		std::mt19937 engine(51);
		std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
		std::normal_distribution<double> normal;

		std::vector<std::complex<float>> slices(batch * n * h);
		for (auto &value : slices)
		{
			value = std::complex<float>(unit(engine), unit(engine));
		}
		std::vector<float> weights(batch * n * h);
		for (auto &value : weights)
		{
			value = 0.5f * (unit(engine) + 1.0f);
		}
		std::vector<float> transforms;
		std::vector<float> ctf;
		for (std::size_t b = 0; b < batch; ++b)
		{
			double w = normal(engine);
			double x = normal(engine);
			double y = normal(engine);
			double z = normal(engine);
			const auto norm = std::sqrt(w * w + x * x + y * y + z * z);
			w /= norm;
			x /= norm;
			y /= norm;
			z /= norm;
			const std::array<double, 12> rows = {{
				1 - 2 * (y * y + z * z), 2 * (x * y - w * z),
				2 * (x * z + w * y), 0,
				2 * (x * y + w * z), 1 - 2 * (x * x + z * z),
				2 * (y * z - w * x), 0,
				2 * (x * z - w * y), 2 * (y * z + w * x),
				1 - 2 * (x * x + y * y), 0
			}};
			for (const auto value : rows)
			{
				transforms.push_back(static_cast<float>(value));
			}
			ctf.insert(ctf.end(), { 15000.0f, 14000.0f, 0.5f, 0.0f });
		}

		m_numerator = make_zeros(
			context, volume, numerical_type::complex_float32
		);
		m_denominator = make_zeros(context, volume, numerical_type::float32);
		m_slices = make_array(context, { batch, n, h }, slices);
		m_weights = make_array(context, { batch, n, h }, weights);
		m_transforms = make_array(context, { batch, 3, 4 }, transforms);
		m_ctf = make_array(context, { batch, 4 }, ctf);
		m_radial = make_array(context, { h }, std::vector<float>(h, 1.0f));
	}

	const std::string& get_description() const noexcept
	{
		return m_description;
	}

	array& get_numerator() noexcept { return m_numerator; }
	array& get_denominator() noexcept { return m_denominator; }
	const array& get_slices() const noexcept { return m_slices; }
	const array& get_weights() const noexcept { return m_weights; }
	const array& get_transforms() const noexcept { return m_transforms; }
	const array& get_ctf() const noexcept { return m_ctf; }
	const array& get_radial() const noexcept { return m_radial; }

private:
	static std::string describe(
		std::size_t slice_extent,
		std::size_t volume_extent,
		std::size_t batch
	)
	{
		std::ostringstream oss;
		oss << "n = " << slice_extent << ", N = " << volume_extent
			<< ", B = " << batch;
		return oss.str();
	}

	std::string m_description;
	array m_numerator;
	array m_denominator;
	array m_slices;
	array m_weights;
	array m_transforms;
	array m_ctf;
	array m_radial;
};

/**
 * @brief Benchmark the three insertions over operands, in place.
 */
void benchmark_insertions(
	const execution_context &context,
	insertion_operands &operands,
	const em::gridding_window &window,
	bool every_operation
)
{
	std::ostringstream suffix;
	suffix << ", " << operands.get_description() << ", "
		<< window.get_profile();

	BENCHMARK("insert_slices" + suffix.str())
	{
		return em::insert_slices(
			operands.get_numerator(),
			operands.get_denominator(),
			operands.get_slices(),
			operands.get_transforms(),
			operands.get_radial(),
			window,
			context,
			&operands.get_numerator(),
			&operands.get_denominator()
		);
	};

	if (!every_operation)
	{
		return;
	}

	BENCHMARK("insert_weighted_slices" + suffix.str())
	{
		return em::insert_weighted_slices(
			operands.get_numerator(),
			operands.get_denominator(),
			operands.get_slices(),
			operands.get_weights(),
			operands.get_transforms(),
			operands.get_radial(),
			window,
			context,
			&operands.get_numerator(),
			&operands.get_denominator()
		);
	};

	BENCHMARK("insert_ctf_slices" + suffix.str())
	{
		return em::insert_ctf_slices(
			operands.get_numerator(),
			operands.get_denominator(),
			operands.get_slices(),
			operands.get_transforms(),
			operands.get_ctf(),
			operands.get_radial(),
			window,
			em::ctf_optics(300, 2.7, 0.1),
			1.0,
			context,
			&operands.get_numerator(),
			&operands.get_denominator()
		);
	};
}

} // anonymous namespace

TEST_CASE(
	"slice insertion over the typical sizes, padded twice",
	"[reconstruction]"
)
{
	const cpu_context context;
	for (const std::size_t n : { 64u, 128u, 256u })
	{
		for (const std::size_t batch : { 64u, 256u })
		{
			insertion_operands operands(context.get(), n, 2 * n, batch);
			for (const auto &window : {
				em::gridding_window::linear(),
				em::gridding_window::kaiser_bessel(2.0, 9.0)
			})
			{
				benchmark_insertions(
					context.get(),
					operands,
					window,
					n == 256 && batch == 64
				);
			}
		}
	}
}

TEST_CASE(
	"slice insertion of 512-pixel images, padded twice",
	"[reconstruction][.large]"
)
{
	// The accumulators take 6.5 GB.
	const cpu_context context;
	insertion_operands operands(context.get(), 512, 1024, 64);
	benchmark_insertions(
		context.get(),
		operands,
		em::gridding_window::linear(),
		false
	);
}
