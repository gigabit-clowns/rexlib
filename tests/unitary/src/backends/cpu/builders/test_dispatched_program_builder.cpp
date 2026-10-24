// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/hardware/command_queue.hpp>

#include "../serial_pool.hpp"

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/operation/operation.hpp>
#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>
#include <rexlib/core/span.hpp>

#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include "../../../core/hardware/mock/mock_memory_resource.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

// A self-contained unary operation over the real types.
REXLIB_DECLARE_OPERATION(
	probe,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::unary_elementwise_shape_rule,
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
 * @tparam Accepts What its accepts_descriptors answers, so that both the
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
	bool accepts_descriptors(
		span<const operand_descriptor>,
		span<const operand_descriptor>
	) const noexcept
	{
		return Accepts;
	}

	probe_plan make_plan(
		const probe_operation&,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor>
	) const
	{
		probe_plan plan;
		plan.rank = output_descriptors[0].get_layout().get_rank();
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
// that the base's own accepts_descriptors is the one answering.
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
		span<const operand_descriptor>,
		span<const operand_descriptor>
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

// An operation whose output type is one of its parameters.
class target_type
{
public:
	explicit target_type(numerical_type type) noexcept
		: m_type(type)
	{
	}

	numerical_type get_type() const noexcept
	{
		return m_type;
	}

private:
	numerical_type m_type;
};

struct type_of
{
	template <typename Parameters>
	static numerical_type get(const Parameters &parameters) noexcept
	{
		return parameters.get_type();
	}
};

REXLIB_DECLARE_PARAMETRIC_OPERATION(
	astype_probe,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::unary_elementwise_shape_rule,
	target_type,
	operand_type_rule<
		type_list<pivot_from_operation<type_of>, pivot_from_input<0>>,
		type_list<slot_same_as<0>>,
		type_list<slot_same_as<1>>
	>
);

template <typename Out, typename In>
class converting_probe_functor
{
public:
	void operator()(
		std::tuple<Out*>,
		std::tuple<const In*>,
		std::tuple<>,
		thread_pool&
	) const
	{
	}
};

class astype_probe_program_builder final
	: public dispatched_program_builder<
		astype_probe_program_builder,
		astype_probe_operation,
		rule_type_dispatcher<typename astype_probe_operation::type_rule>
	>
{
public:
	probe_plan make_plan(
		const astype_probe_operation&,
		span<const operand_descriptor>,
		span<const operand_descriptor>
	) const
	{
		return probe_plan();
	}

	template <typename Out, typename In>
	converting_probe_functor<Out, In> make_loop_functor(
		const astype_probe_operation&,
		probe_plan&,
		type_list<Out>,
		type_list<In>
	) const
	{
		return converting_probe_functor<Out, In>();
	}
};

operand_descriptor make_operand_descriptor(
	const memory_resource *resource,
	numerical_type type = numerical_type::float32
)
{
	const std::array<std::size_t, 1> extents { 4 };
	return operand_descriptor(
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
	// already state does not hide accepts_descriptors, and the base's own
	// answer lets the operands through.
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const silent_program_builder builder;
	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource)
	};

	CHECK(
		builder.get_suitability(
			operation, make_span(outputs), make_span(inputs), queue
		) != backend_priority::unsupported
	);
}

TEST_CASE(
	"dispatched_program_builder declines when the family declines the "
	"descriptors",
	"[dispatched_program_builder]"
)
{
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const probe_operation operation;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource)
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
	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource),
		make_operand_descriptor(&host_resource)
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

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&device_resource)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&device_resource)
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

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(nullptr)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(nullptr)
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

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(nullptr),
		make_operand_descriptor(nullptr)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(nullptr)
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

TEST_CASE(
	"dispatched_program_builder resolves the data types the operation "
	"supplies",
	"[dispatched_program_builder]"
)
{
	mock_memory_resource host_resource;
	ALLOW_CALL(host_resource, get_kind())
		.RETURN(memory_resource_kind::host);

	const astype_probe_program_builder builder;
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(&host_resource, numerical_type::float64)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(&host_resource, numerical_type::int32)
	};

	const astype_probe_operation matching(numerical_type::float64);
	CHECK(
		builder.get_suitability(
			matching, make_span(outputs), make_span(inputs), queue
		) != backend_priority::unsupported
	);

	const astype_probe_operation other(numerical_type::float32);
	CHECK(
		builder.get_suitability(
			other, make_span(outputs), make_span(inputs), queue
		) == backend_priority::unsupported
	);
}

TEST_CASE(
	"dispatched_program_builder builds with the data types the operation "
	"supplies",
	"[dispatched_program_builder]"
)
{
	const astype_probe_program_builder builder;
	const astype_probe_operation operation(numerical_type::float64);
	cpu::command_queue queue(get_serial_pool());

	const std::vector<operand_descriptor> outputs {
		make_operand_descriptor(nullptr, numerical_type::float64)
	};
	const std::vector<operand_descriptor> inputs {
		make_operand_descriptor(nullptr, numerical_type::int32)
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
