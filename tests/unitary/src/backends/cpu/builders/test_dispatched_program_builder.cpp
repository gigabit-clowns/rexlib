// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>

#include <xmipp4/backends/cpu/thread_pool.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include "../serial_pool.hpp"

#include <xmipp4/core/dispatch/basic_operation.hpp>
#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/dispatch/operation.hpp>
#include <xmipp4/core/hardware/memory_resource_kind.hpp>
#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/span.hpp>

#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/elementwise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

#include "../../../core/hardware/mock/mock_memory_resource.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace xmipp4;
using namespace xmipp4::cpu;

namespace
{

// A self-contained unary operation over the real types.
XMIPP4_DECLARE_OPERATION(
	probe,
	ops::ops_component,
	XMIPP4_OPERANDS("result"),
	XMIPP4_OPERANDS("value"),
	ops::elementwise_operation_shape_policy,
	ops::unary_homogeneous_rule<>
);

// An observable stand-in for a family's plan.
struct probe_plan
{
	std::size_t rank = 0;
};

template <typename T>
class probe_functor
{
public:
	explicit probe_functor(probe_plan plan)
		: m_plan(plan)
	{
	}

	void operator()(
		std::tuple<T*>,
		std::tuple<const T*>,
		std::tuple<>,
		thread_pool&
	) const
	{
	}

private:
	probe_plan m_plan;
};

/**
 * @brief A minimal family, to exercise the base rather than any real one.
 *
 * @tparam Accepts What its accepts_signatures answers, so that both the
 * accepting and the declining branch can be reached.
 */
template <bool Accepts>
class probe_program_builder final
	: public dispatched_program_builder<
		probe_program_builder<Accepts>,
		probe_operation,
		rule_type_dispatcher<typename probe_operation::type_rule>
	>
{
public:
	bool accepts_signatures(
		span<const operand_signature>,
		span<const operand_signature>
	) const noexcept
	{
		return Accepts;
	}

	probe_plan make_plan(
		const probe_operation&,
		span<const operand_signature> output_signatures,
		span<const operand_signature>
	) const
	{
		probe_plan plan;
		plan.rank = output_signatures[0].get_layout().get_rank();
		return plan;
	}

	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const probe_operation&,
		probe_plan &plan,
		type_list<Outs...>,
		type_list<Ins...>
	) const
	{
		using out_t = typename type_list_element<0, type_list<Outs...>>::type;
		return probe_functor<out_t>(plan);
	}
};

// A family that says nothing beyond its operand counts and typing rule, so
// that the base's own accepts_signatures is the one answering.
class silent_program_builder final
	: public dispatched_program_builder<
		silent_program_builder,
		probe_operation,
		rule_type_dispatcher<typename probe_operation::type_rule>
	>
{
public:
	probe_plan make_plan(
		const probe_operation&,
		span<const operand_signature>,
		span<const operand_signature>
	) const
	{
		return probe_plan();
	}

	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const probe_operation&,
		probe_plan &plan,
		type_list<Outs...>,
		type_list<Ins...>
	) const
	{
		using out_t = typename type_list_element<0, type_list<Outs...>>::type;
		return probe_functor<out_t>(plan);
	}
};

operand_signature make_signature(
	const memory_resource *resource,
	numerical_type type = numerical_type::float32
)
{
	const std::array<std::size_t, 1> extents { 4 };
	return operand_signature(
		strided_layout::make_contiguous_layout(make_span(extents)),
		type,
		resource
	);
}

} // anonymous namespace

TEST_CASE(
	"dispatched_program_builder reports the operation it targets",
	"[dispatched_program_builder]"
)
{
	const silent_program_builder builder;

	CHECK( builder.get_operation_id() == operation_id::of<probe_operation>() );
}

TEST_CASE(
	"dispatched_program_builder accepts by default",
	"[dispatched_program_builder]"
)
{
	// A family whose whole requirement its operand counts and typing rule
	// already state does not hide accepts_signatures, and the base's own
	// answer lets the operands through.
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const silent_program_builder builder;
	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(&host_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&host_resource)
	};

	CHECK(
		builder.get_suitability(
			operation, make_span(outputs), make_span(inputs), queue
		) != backend_priority::unsupported
	);
}

TEST_CASE(
	"dispatched_program_builder declines when the family declines the "
	"signatures",
	"[dispatched_program_builder]"
)
{
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(&host_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&host_resource)
	};

	// The same operands, differing only in what the family says about them.
	const probe_program_builder<true> accepting;
	const probe_program_builder<false> declining;

	CHECK(
		accepting.get_suitability(
			operation, make_span(outputs), make_span(inputs), queue
		) != backend_priority::unsupported
	);
	CHECK(
		declining.get_suitability(
			operation, make_span(outputs), make_span(inputs), queue
		) == backend_priority::unsupported
	);
}

TEST_CASE(
	"dispatched_program_builder declines an operand count it cannot serve",
	"[dispatched_program_builder]"
)
{
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const silent_program_builder builder;
	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	// The operation takes exactly one input.
	const std::vector<operand_signature> outputs {
		make_signature(&host_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&host_resource),
		make_signature(&host_resource)
	};

	CHECK(
		builder.get_suitability(
			operation, make_span(outputs), make_span(inputs), queue
		) == backend_priority::unsupported
	);
}

TEST_CASE(
	"dispatched_program_builder declines an operand not on the host",
	"[dispatched_program_builder]"
)
{
	mock_memory_resource device_resource;
	ALLOW_CALL(device_resource, get_kind())
		.RETURN(memory_resource_kind::device_local);

	const silent_program_builder builder;
	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(&device_resource)
	};
	const std::vector<operand_signature> inputs {
		make_signature(&device_resource)
	};

	CHECK(
		builder.get_suitability(
			operation, make_span(outputs), make_span(inputs), queue
		) == backend_priority::unsupported
	);
}

TEST_CASE(
	"dispatched_program_builder builds through the family's plan and functor",
	"[dispatched_program_builder]"
)
{
	const silent_program_builder builder;
	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(nullptr)
	};
	const std::vector<operand_signature> inputs {
		make_signature(nullptr)
	};

	const auto program = builder.build(
		operation,
		make_span(outputs),
		make_span(inputs),
		queue,
		nullptr
	);

	CHECK( program != nullptr );
}

TEST_CASE(
	"dispatched_program_builder names the operation when the operand count "
	"is wrong",
	"[dispatched_program_builder]"
)
{
	const silent_program_builder builder;
	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_signature> outputs {
		make_signature(nullptr),
		make_signature(nullptr)
	};
	const std::vector<operand_signature> inputs {
		make_signature(nullptr)
	};

	// One base serves every family, so the message says which operation was
	// asked for rather than which family was asked.
	try
	{
		builder.build(
			operation,
			make_span(outputs),
			make_span(inputs),
			queue,
			nullptr
		);
		FAIL( "build did not reject the operand count" );
	}
	catch (const std::invalid_argument &error)
	{
		const std::string message = error.what();
		CHECK( message.find("probe") != std::string::npos );
		CHECK( message.find("output") != std::string::npos );
	}
}
