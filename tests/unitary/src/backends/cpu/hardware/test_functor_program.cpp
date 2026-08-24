// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/hardware/functor_program.hpp>

#include <rex/backends/cpu/thread_pool.hpp>

#include "../serial_pool.hpp"
#include <core/hardware/host_memory/host_buffer.hpp>

#include <rex/core/hardware/buffer.hpp>
#include <rex/core/hardware/program_scratch_requirement.hpp>
#include <rex/core/hardware/memory_resource_affinity.hpp>
#include <rex/core/span.hpp>

#include "../../../core/hardware/mock/mock_buffer.hpp"

#include <trompeloeil.hpp>

#include <array>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

// Mock called with a float output, a heterogeneous (float, int) input pair
// and an int scratch operand.
class mock_typed_call
{
public:
	MAKE_MOCK3(
		call,
		void(
			std::tuple<float*>,
			std::tuple<const float*, const int*>,
			std::tuple<int*>
		)
	);
};

// Mock called with type-erased (void) operands.
class mock_void_call
{
public:
	MAKE_MOCK3(
		call,
		void(std::tuple<void*>, std::tuple<const void*>, std::tuple<>)
	);
};

// Copyable functor forwarding to a trompeloeil mock. The program stores the
// functor by value, so it cannot hold the (non-copyable) mock directly.
template <typename Mock>
struct mock_functor
{
	Mock *mock;

	template <typename Outputs, typename Inputs, typename Scratches>
	void operator()(
		Outputs &&outputs,
		Inputs &&inputs,
		Scratches &&scratch,
		thread_pool &/*pool*/
	) const
	{
		mock->call(
			std::forward<Outputs>(outputs),
			std::forward<Inputs>(inputs),
			std::forward<Scratches>(scratch)
		);
	}
};

// No-op functor callable with any operands, for cases where execute must throw
// before the functor is invoked.
struct noop_functor
{
	template <typename... Ts>
	void operator()(Ts&&...) const
	{
	}
};

std::shared_ptr<host_buffer> make_host_buffer(std::size_t size)
{
	return std::make_shared<host_buffer>(size, 64);
}


} // namespace


TEST_CASE(
	"make_functor_program without scratch reports no scratch requirements",
	"[cpu::functor_program]"
)
{
	const auto program = make_functor_program(
		noop_functor(),
		type_list<float>(),
		type_list<float, int>()
	);

	CHECK( program->get_scratch_requirements().empty() );
}

TEST_CASE(
	"make_functor_program with scratch exposes the given requirements",
	"[cpu::functor_program]"
)
{
	const auto program = make_functor_program(
		noop_functor(),
		type_list<int>(),
		type_list<int>(),
		type_list<int>(),
		std::array<program_scratch_requirement, 1>{{
			program_scratch_requirement(
				sizeof(int),
				alignof(int),
				memory_resource_affinity::host
			)
		}}
	);

	const auto requirements = program->get_scratch_requirements();

	REQUIRE( requirements.size() == 1 );
	CHECK( requirements[0].get_size() == sizeof(int) );
	CHECK( requirements[0].get_alignment() == alignof(int) );
	CHECK( requirements[0].get_affinity() == memory_resource_affinity::host );
}

TEST_CASE(
	"cpu::functor_program::execute forwards typed host pointers to the functor",
	"[cpu::functor_program]"
)
{
	using trompeloeil::_;

	mock_typed_call mock;
	const auto program = make_functor_program(
		mock_functor<mock_typed_call>{ &mock },
		type_list<float>(),
		type_list<float, int>(),
		type_list<int>(),
		std::array<program_scratch_requirement, 1>{{
			program_scratch_requirement(
				sizeof(int),
				alignof(int),
				memory_resource_affinity::host
			)
		}}
	);

	const auto output = make_host_buffer(sizeof(float));
	const auto input_a = make_host_buffer(sizeof(float));
	const auto input_b = make_host_buffer(sizeof(int));
	const auto scratch_buffer = make_host_buffer(sizeof(int));

	auto *output_ptr = static_cast<float*>(output->get_host_ptr());
	const auto *input_a_ptr = static_cast<float*>(input_a->get_host_ptr());
	const auto *input_b_ptr = static_cast<int*>(input_b->get_host_ptr());
	auto *scratch_ptr = static_cast<int*>(scratch_buffer->get_host_ptr());

	const std::vector<std::shared_ptr<buffer>> outputs = { output };
	const std::vector<std::shared_ptr<const buffer>> inputs =
		{ input_a, input_b };
	const std::vector<std::shared_ptr<buffer>> scratch = { scratch_buffer };

	// The functor receives one typed pointer per operand, with inputs const
	// qualified, in the declared order.
	REQUIRE_CALL(mock, call(_, _, _))
		.WITH(
			std::get<0>(_1) == output_ptr &&
			std::get<0>(_2) == input_a_ptr &&
			std::get<1>(_2) == input_b_ptr &&
			std::get<0>(_3) == scratch_ptr
		);

	program->execute(
		make_span(outputs),
		make_span(inputs),
		make_span(scratch),
		*get_serial_pool()
	);
}

TEST_CASE(
	"cpu::functor_program::execute forwards void operands as void pointers",
	"[cpu::functor_program]"
)
{
	using trompeloeil::_;

	mock_void_call mock;
	const auto program = make_functor_program(
		mock_functor<mock_void_call>{ &mock },
		type_list<void>(),
		type_list<void>()
	);

	const auto output = make_host_buffer(sizeof(int));
	const auto input = make_host_buffer(sizeof(int));

	void *output_ptr = output->get_host_ptr();
	const void *input_ptr = input->get_host_ptr();

	const std::vector<std::shared_ptr<buffer>> outputs = { output };
	const std::vector<std::shared_ptr<const buffer>> inputs = { input };
	const std::vector<std::shared_ptr<buffer>> scratch;

	REQUIRE_CALL(mock, call(_, _, _))
		.WITH(
			std::get<0>(_1) == output_ptr &&
			std::get<0>(_2) == input_ptr
		);

	program->execute(
		make_span(outputs),
		make_span(inputs),
		make_span(scratch),
		*get_serial_pool()
	);
}

TEST_CASE(
	"cpu::functor_program::execute throws when a buffer is not host accessible",
	"[cpu::functor_program]"
)
{
	const auto program = make_functor_program(
		noop_functor(),
		type_list<float>(),
		type_list<>()
	);

	const auto output = std::make_shared<mock_buffer>();
	ALLOW_CALL(*output, get_host_ptr())
		.RETURN(nullptr);

	const std::vector<std::shared_ptr<buffer>> outputs = { output };
	const std::vector<std::shared_ptr<const buffer>> inputs;
	const std::vector<std::shared_ptr<buffer>> scratch;

	CHECK_THROWS_AS(
		program->execute(
			make_span(outputs),
			make_span(inputs),
			make_span(scratch),
			*get_serial_pool()
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"cpu::functor_program::execute throws when the operand count is wrong",
	"[cpu::functor_program]"
)
{
	const auto program = make_functor_program(
		noop_functor(),
		type_list<float>(),
		type_list<>()
	);

	// The program expects one output but none is supplied.
	const std::vector<std::shared_ptr<buffer>> outputs;
	const std::vector<std::shared_ptr<const buffer>> inputs;
	const std::vector<std::shared_ptr<buffer>> scratch;

	CHECK_THROWS_AS(
		program->execute(
			make_span(outputs),
			make_span(inputs),
			make_span(scratch),
			*get_serial_pool()
		),
		std::invalid_argument
	);
}
