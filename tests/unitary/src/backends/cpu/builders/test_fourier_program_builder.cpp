// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <xmipp4/backends/cpu/program.hpp>
#include <xmipp4/backends/cpu/thread_pool.hpp>

#include "../serial_pool.hpp"
#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/hardware/buffer.hpp>
#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/numerical/fixed_width_float.hpp>
#include <xmipp4/core/hardware/memory_resource_kind.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/span.hpp>

#include <xmipp4/ops/fourier/fft_operation.hpp>
#include <xmipp4/ops/policies/axis_list.hpp>

#include "../../../core/hardware/mock/mock_memory_resource.hpp"

#include <complex>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::cpu;

namespace
{


using complex_type = std::complex<float32_t>;

using fft_builder = fourier_program_builder<
	ops::fft_operation,
	forward_complex_fourier_transform
>;

operand_signature make_signature(
	std::vector<std::size_t> extents,
	std::vector<std::ptrdiff_t> strides,
	std::ptrdiff_t offset,
	numerical_type data_type,
	const memory_resource *resource = nullptr
)
{
	return operand_signature(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			offset
		),
		data_type,
		resource
	);
}

template <typename T>
std::shared_ptr<host_buffer> make_buffer(std::size_t count)
{
	return std::make_shared<host_buffer>(count * sizeof(T), alignof(T));
}

/**
 * @brief Run a program over one input buffer and one output buffer.
 */
void run(
	const std::shared_ptr<xmipp4::program> &program,
	const std::shared_ptr<host_buffer> &output,
	const std::shared_ptr<host_buffer> &input,
	thread_pool &pool = *get_serial_pool()
)
{
	const std::vector<std::shared_ptr<buffer>> outputs { output };
	const std::vector<std::shared_ptr<const buffer>> inputs { input };
	const std::vector<std::shared_ptr<buffer>> scratches;

	auto &executable = dynamic_cast<cpu::program&>(*program);
	executable.execute(
		make_span(outputs),
		make_span(inputs),
		make_span(scratches),
		pool
	);
}

/**
 * @brief Check a spectrum against the transform of {1, 2, 3, 4}.
 *
 * @param data The buffer holding it.
 * @param offset Where the spectrum starts, in elements.
 * @param stride The step between its values, in elements.
 */
void check_ramp_spectrum(
	const complex_type *data,
	std::ptrdiff_t offset,
	std::ptrdiff_t stride
)
{
	const complex_type expected[] = {
		complex_type(10.0F, 0.0F),
		complex_type(-2.0F, 2.0F),
		complex_type(-2.0F, 0.0F),
		complex_type(-2.0F, -2.0F)
	};

	for (std::ptrdiff_t i = 0; i < 4; ++i)
	{
		INFO( "coefficient " << i );
		const auto value = data[offset + i*stride];
		CHECK( std::abs(value - expected[i]) < 1e-4F );
	}
}

// The signature of the transform every case below performs: four samples
// along the only axis there is.
const std::vector<std::size_t> unit_extents = { 4 };

} // namespace

TEST_CASE(
	"fourier_program_builder reports its target operation id",
	"[fourier_program_builder]"
)
{
	const fft_builder builder;
	CHECK(
		builder.get_operation_id() == operation_id::of<ops::fft_operation>()
	);
}

TEST_CASE(
	"fourier_program_builder starts each operand where it is offset to",
	"[fourier_program_builder]"
)
{
	// An operand is a view onto storage rather than the whole of it, so what
	// the transform is handed is a base pointer and an offset, and it has to
	// displace one by the other. Nothing an array verb builds today is
	// offset, which is why this is pinned here rather than through one.
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(unit_extents, { 1 }, 2, numerical_type::complex_float32)
	};
	const std::vector<operand_signature> inputs {
		make_signature(unit_extents, { 1 }, 3, numerical_type::float32)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto output_buffer = make_buffer<complex_type>(8);
	const auto input_buffer = make_buffer<float32_t>(8);
	auto *output_data = static_cast<complex_type*>(
		output_buffer->get_host_ptr()
	);
	auto *input_data = static_cast<float32_t*>(input_buffer->get_host_ptr());

	const complex_type sentinel(-1.0F, -1.0F);
	for (std::size_t i = 0; i < 8; ++i)
	{
		output_data[i] = sentinel;
		input_data[i] = 0.0F;
	}
	for (std::size_t i = 0; i < 4; ++i)
	{
		input_data[3 + i] = static_cast<float32_t>(i + 1);
	}

	run(program, output_buffer, input_buffer);

	check_ramp_spectrum(output_data, 2, 1);

	// What lies outside the operand is not the operand's to write.
	CHECK( output_data[0] == sentinel );
	CHECK( output_data[1] == sentinel );
	CHECK( output_data[6] == sentinel );
	CHECK( output_data[7] == sentinel );
}

TEST_CASE(
	"fourier_program_builder transforms an operand that is not contiguous",
	"[fourier_program_builder]"
)
{
	// The transform is described to pocketfft by strides rather than by a
	// promise of contiguity, so a signal every other element apart is as
	// transformable as one laid out end to end.
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(unit_extents, { 2 }, 1, numerical_type::complex_float32)
	};
	const std::vector<operand_signature> inputs {
		make_signature(unit_extents, { 2 }, 0, numerical_type::float32)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto output_buffer = make_buffer<complex_type>(8);
	const auto input_buffer = make_buffer<float32_t>(8);
	auto *output_data = static_cast<complex_type*>(
		output_buffer->get_host_ptr()
	);
	auto *input_data = static_cast<float32_t*>(input_buffer->get_host_ptr());

	for (std::size_t i = 0; i < 8; ++i)
	{
		output_data[i] = complex_type(0.0F, 0.0F);
		input_data[i] = 0.0F;
	}
	for (std::size_t i = 0; i < 4; ++i)
	{
		input_data[2*i] = static_cast<float32_t>(i + 1);
	}

	run(program, output_buffer, input_buffer);

	check_ramp_spectrum(output_data, 1, 2);
}

TEST_CASE(
	"fourier_program_builder transforms a complex operand where it lies",
	"[fourier_program_builder]"
)
{
	// A complex operand needs no widening, so the transform reads the input
	// directly instead of going through the output.
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(unit_extents, { 1 }, 0, numerical_type::complex_float32)
	};
	const std::vector<operand_signature> inputs {
		make_signature(unit_extents, { 1 }, 0, numerical_type::complex_float32)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto output_buffer = make_buffer<complex_type>(4);
	const auto input_buffer = make_buffer<complex_type>(4);
	auto *output_data = static_cast<complex_type*>(
		output_buffer->get_host_ptr()
	);
	auto *input_data = static_cast<complex_type*>(input_buffer->get_host_ptr());

	for (std::size_t i = 0; i < 4; ++i)
	{
		output_data[i] = complex_type(0.0F, 0.0F);
		input_data[i] = complex_type(static_cast<float32_t>(i + 1), 0.0F);
	}

	run(program, output_buffer, input_buffer);

	check_ramp_spectrum(output_data, 0, 1);
}

TEST_CASE(
	"fourier_program_builder reports half precision unsupported",
	"[fourier_program_builder]"
)
{
	// The operation admits it and this backend does not, so the answer is a
	// refusal to build rather than a failure to, which is what lets the
	// manager look for a backend that can.
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const std::vector<operand_signature> outputs {
		make_signature(
			unit_extents, { 1 }, 0,
			numerical_type::complex_float16, &host_resource
		)
	};
	const std::vector<operand_signature> inputs {
		make_signature(
			unit_extents, { 1 }, 0,
			numerical_type::float16, &host_resource
		)
	};

	CHECK(
		builder.get_suitability(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue
		) == backend_priority::unsupported
	);
}

TEST_CASE(
	"fourier_program_builder builds for the precisions it does have",
	"[fourier_program_builder]"
)
{
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const std::vector<operand_signature> outputs {
		make_signature(
			unit_extents, { 1 }, 0,
			numerical_type::complex_float64, &host_resource
		)
	};
	const std::vector<operand_signature> inputs {
		make_signature(
			unit_extents, { 1 }, 0,
			numerical_type::float64, &host_resource
		)
	};

	CHECK(
		builder.get_suitability(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue
		) != backend_priority::unsupported
	);
}

TEST_CASE(
	"fourier_program_builder::build throws on a wrong operand count",
	"[fourier_program_builder]"
)
{
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	// A transform produces exactly one operand.
	const std::vector<operand_signature> outputs {
		make_signature(unit_extents, { 1 }, 0, numerical_type::complex_float32),
		make_signature(unit_extents, { 1 }, 0, numerical_type::complex_float32)
	};
	const std::vector<operand_signature> inputs {
		make_signature(unit_extents, { 1 }, 0, numerical_type::float32)
	};

	CHECK_THROWS_AS(
		builder.build(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue,
			nullptr
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"fourier_program_builder transforms an operand laid out backwards",
	"[fourier_program_builder]"
)
{
	// A stride is a signed step, so an operand running back to front through
	// its storage is described the same way as any other and needs no copy
	// into forward order first.
	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 0 } };
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(unit_extents, { 1 }, 0, numerical_type::complex_float32)
	};
	const std::vector<operand_signature> inputs {
		make_signature(unit_extents, { -1 }, 3, numerical_type::float32)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto output_buffer = make_buffer<complex_type>(4);
	const auto input_buffer = make_buffer<float32_t>(4);
	auto *output_data = static_cast<complex_type*>(
		output_buffer->get_host_ptr()
	);
	auto *input_data = static_cast<float32_t*>(input_buffer->get_host_ptr());

	// Stored backwards, so that read backwards it is the same ramp.
	for (std::size_t i = 0; i < 4; ++i)
	{
		output_data[i] = complex_type(0.0F, 0.0F);
		input_data[i] = static_cast<float32_t>(4 - i);
	}

	run(program, output_buffer, input_buffer);

	check_ramp_spectrum(output_data, 0, 1);
}

TEST_CASE(
	"fourier_program_builder transforms the same over threads as over one",
	"[fourier_program_builder]"
)
{
	// pocketfft threads a transform from a pool of its own, and every other
	// case here is too small to make it want to: it spreads a transform over
	// however many of them it is offered, but only once the count of
	// transforms divided by the vector width, and quartered again for an axis
	// under a thousand samples, comes to more than one. A four sample signal
	// leaves it at one thread whatever it is offered, so nothing else here
	// reaches the threaded path at all.
	//
	// Five hundred and twelve rows of sixty four samples do, and by enough of
	// a margin to still do it at the widest vector the header compiles for,
	// which is what keeps this from depending on what the machine supports.
	constexpr std::size_t row_count = 512;
	constexpr std::size_t sample_count = 64;
	constexpr std::size_t count = row_count * sample_count;

	const fft_builder builder;
	const ops::fft_operation operation{ ops::axis_list{ 1 } };
	cpu::command_queue queue(get_serial_pool());

	const std::vector<std::size_t> extents = { row_count, sample_count };
	const std::vector<std::ptrdiff_t> strides = {
		static_cast<std::ptrdiff_t>(sample_count),
		1
	};

	const std::vector<operand_signature> outputs {
		make_signature(extents, strides, 0, numerical_type::complex_float32)
	};
	const std::vector<operand_signature> inputs {
		make_signature(extents, strides, 0, numerical_type::complex_float32)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto input_buffer = make_buffer<complex_type>(count);
	auto *input_data = static_cast<complex_type*>(
		input_buffer->get_host_ptr()
	);

	// One sample per row, so that every coefficient of that row transforms to
	// the same value and a row is told apart by it. A chunk that reads or
	// writes the wrong rows shows up as a value belonging to another one,
	// which a signal that is the same in every row could not reveal.
	for (std::size_t i = 0; i < count; ++i)
	{
		input_data[i] = complex_type(0.0F, 0.0F);
	}
	for (std::size_t row = 0; row < row_count; ++row)
	{
		input_data[row*sample_count] =
			complex_type(static_cast<float32_t>(row + 1), 0.0F);
	}

	const auto threaded_buffer = make_buffer<complex_type>(count);
	const auto serial_buffer = make_buffer<complex_type>(count);
	auto *threaded = static_cast<complex_type*>(
		threaded_buffer->get_host_ptr()
	);
	auto *serial = static_cast<complex_type*>(serial_buffer->get_host_ptr());

	thread_pool pool(3);
	REQUIRE( pool.get_size() > 1 );
	run(program, threaded_buffer, input_buffer, pool);
	run(program, serial_buffer, input_buffer, *get_serial_pool());

	// The transform of a lone sample is that sample at every coefficient, so
	// every one of the values below is known ahead of the run rather than
	// taken from whatever the other pool produced.
	std::size_t threaded_mismatches = 0;
	std::size_t serial_mismatches = 0;
	std::size_t disagreements = 0;
	for (std::size_t row = 0; row < row_count; ++row)
	{
		const complex_type expected(static_cast<float32_t>(row + 1), 0.0F);
		for (std::size_t i = 0; i < sample_count; ++i)
		{
			const auto index = row*sample_count + i;
			if (std::abs(threaded[index] - expected) >= 1e-3F)
			{
				++threaded_mismatches;
			}
			if (std::abs(serial[index] - expected) >= 1e-3F)
			{
				++serial_mismatches;
			}
			if (threaded[index] != serial[index])
			{
				++disagreements;
			}
		}
	}

	CHECK( threaded_mismatches == 0 );
	CHECK( serial_mismatches == 0 );

	// Splitting a transform decides which coefficients take the vector path
	// and which the scalar tail, so this is allowed to be a tolerance rather
	// than an equality. It is stated as one because it has held so far, and a
	// failure here is worth looking at before it is loosened.
	CHECK( disagreements == 0 );
}
