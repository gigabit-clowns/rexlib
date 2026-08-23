// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rex/core/dispatch/dispatcher.hpp>

#include <rex/core/dispatch/program_manager.hpp>
#include <rex/core/dispatch/program_cache.hpp>
#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/dispatch/operation_id.hpp>
#include <rex/core/dispatch/operation_arity.hpp>
#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/backend_priority.hpp>

#include <rex/core/hardware/device_context.hpp>
#include <rex/core/hardware/device_session.hpp>
#include <rex/core/hardware/device.hpp>
#include <rex/core/hardware/device_properties.hpp>
#include <rex/core/hardware/memory_allocator.hpp>
#include <rex/core/hardware/command_queue.hpp>
#include <rex/core/hardware/command.hpp>
#include <rex/core/hardware/program.hpp>
#include <rex/core/hardware/program_scratch_requirement.hpp>

#include "mock/mock_operation.hpp"
#include "mock/mock_operation_shape_policy.hpp"
#include "mock/mock_operation_data_type_policy.hpp"
#include "mock/mock_program_builder.hpp"
#include "../hardware/mock/mock_device.hpp"
#include "../hardware/mock/mock_memory_resource.hpp"
#include "../hardware/mock/mock_memory_allocator.hpp"
#include "../hardware/mock/mock_command_queue.hpp"
#include "../hardware/mock/mock_buffer.hpp"
#include "../hardware/mock/mock_program.hpp"

#include <trompeloeil.hpp>

#include <memory>
#include <utility>
#include <stdexcept>
#include <vector>

using namespace rex;

namespace
{

using shape_type = operation_shape_policy::shape_type;

array_descriptor make_descriptor(const shape_type &shape, numerical_type type)
{
	return array_descriptor(
		strided_layout::make_contiguous_layout(make_span(shape)),
		type
	);
}

class eager_dispatcher_fixture
{
public:
	eager_dispatcher_fixture()
		: device(std::make_shared<mock_device>())
		, host_allocator(std::make_shared<mock_memory_allocator>())
		, device_allocator(std::make_shared<mock_memory_allocator>())
		, default_queue(std::make_shared<mock_command_queue>())
		, manager(std::make_shared<program_manager>())
		, program(std::make_shared<mock_program>())
	{

		REQUIRE_CALL(
			*device,
			get_memory_resource(memory_resource_affinity::host)
		)
			.LR_RETURN(host_resource);
		REQUIRE_CALL(
			*device,
			get_memory_resource(memory_resource_affinity::device)
		)
			.LR_RETURN(device_resource);
		REQUIRE_CALL(host_resource, create_allocator())
			.RETURN(host_allocator);
		REQUIRE_CALL(device_resource, create_allocator())
			.RETURN(device_allocator);
		REQUIRE_CALL(*device, create_command_queue())
			.RETURN(default_queue);

		device_properties properties;
		properties.set_optimal_data_alignment(64);

		session = std::make_shared<device_session>(
			device,
			std::move(properties)
		);
		context = device_context(session);

		eager_dispatcher = make_eager_dispatcher(manager);

		// The output alignment query is harmless for tests that never allocate.
		expectations.push_back(
			NAMED_ALLOW_CALL(*device_allocator, get_max_alignment())
				.RETURN(std::size_t(256))
		);
	}

protected:
	// The dispatcher hands the operation's description to the policies, so
	// that whichever of them rejects a launch can name the operand.
	static const operation_descriptor& mock_descriptor()
	{
		static REX_CONST_CONSTEXPR auto outputs =
			make_operand_names("result");
		static REX_CONST_CONSTEXPR auto inputs =
			make_operand_names("value");
		static const operation_descriptor instance =
			make_operation_descriptor("xmipp4.test", "mock", outputs, inputs);
		return instance;
	}

	// Set up @ref op as a unary operation whose policies deduce the given
	// output shape and data type from its single input.
	void expect_unary_operation(shape_type out_shape, numerical_type out_type)
	{
		expectations.push_back(
			NAMED_ALLOW_CALL(op, get_arity())
				.RETURN(operation_arity::unary())
		);
		expectations.push_back(
			NAMED_ALLOW_CALL(op, get_descriptor())
				.LR_RETURN(mock_descriptor())
		);
		expectations.push_back(
			NAMED_ALLOW_CALL(op, get_operation_shape_policy())
				.LR_RETURN(shape_policy)
		);
		expectations.push_back(
			NAMED_ALLOW_CALL(op, get_operation_data_type_policy())
				.LR_RETURN(dtype_policy)
		);
		expectations.push_back(
			NAMED_REQUIRE_CALL(
				shape_policy,
				deduce(
					ANY(const operation_descriptor&),
					ANY(span<shape_type>),
					ANY(span<const shape_type>)
				)
			)
				.SIDE_EFFECT(_2[0] = out_shape)
		);
		expectations.push_back(
			NAMED_REQUIRE_CALL(
				dtype_policy,
				deduce(
					ANY(const operation_descriptor&),
					ANY(span<numerical_type>),
					ANY(span<const numerical_type>)
				)
			)
				.SIDE_EFFECT(_2[0] = out_type)
		);
	}

	// Expect both policies to be queried for admissibility. This only happens
	// when at least one output is pre-allocated; the dispatcher delegates the
	// decision to the policies, so the mocks simply accept (they do not throw)
	// regardless of the arguments.
	void expect_policies_accept()
	{
		expectations.push_back(
			NAMED_REQUIRE_CALL(
				shape_policy,
				accept(
					ANY(const operation_descriptor&),
					ANY(span<const shape_type>),
					ANY(span<const shape_type>),
					ANY(span<const shape_type>)
				)
			)
		);
		expectations.push_back(
			NAMED_REQUIRE_CALL(
				dtype_policy,
				accept(
					ANY(const operation_descriptor&),
					ANY(span<const numerical_type>),
					ANY(span<const numerical_type>),
					ANY(span<const numerical_type>)
				)
			)
		);
	}

	// Register a builder so that the program manager resolves @ref program for
	// @ref op.
	void register_program_builder()
	{
		auto builder = std::make_unique<mock_program_builder>();
		expectations.push_back(
			NAMED_REQUIRE_CALL(*builder, get_operation_id())
				.RETURN(operation_id::of<mock_operation>())
		);
		expectations.push_back(
			NAMED_REQUIRE_CALL(
				*builder,
				get_suitability(
					ANY(const operation&),
					ANY(span<const operand_signature>),
					ANY(span<const operand_signature>),
					ANY(command_queue&)
				)
			)
				.RETURN(backend_priority::normal)
		);
		expectations.push_back(
			NAMED_REQUIRE_CALL(
				*builder,
				build(
					ANY(const operation&),
					ANY(span<const operand_signature>),
					ANY(span<const operand_signature>),
					ANY(command_queue&),
					ANY(program_cache*)
				)
			)
				.LR_RETURN(program)
		);
		REQUIRE(manager->register_builder(std::move(builder)));
	}

	// Make @ref program report the given scratch requirements (none by default).
	void expect_program_scratch(
		span<const program_scratch_requirement> requirements = {}
	)
	{
		expectations.push_back(
			NAMED_REQUIRE_CALL(*program, get_scratch_requirements())
				.RETURN(requirements)
		);
	}

	// Create a tracked mock buffer that reports the device memory resource.
	std::shared_ptr<mock_buffer> make_buffer()
	{
		auto buffer = std::make_shared<mock_buffer>();
		expectations.push_back(
			NAMED_ALLOW_CALL(*buffer, get_memory_resource())
				.LR_RETURN(device_resource)
		);
		buffers.push_back(buffer);
		return buffer;
	}

	// Build a read-only input operand backed by freshly allocated storage.
	const_array make_input_with_storage(
		const shape_type &shape,
		numerical_type type
	)
	{
		array input_array(make_buffer(), make_descriptor(shape, type));
		return input_array.share_const();
	}

	// Expect a single command submission onto the active queue, carrying
	// @ref program and the given output, input and scratch buffers.
	void expect_command_submission(
		std::shared_ptr<buffer> output_buffer,
		std::shared_ptr<const buffer> input_storage,
		std::shared_ptr<buffer> scratch_buffer = nullptr
	)
	{
		auto &queue = static_cast<mock_command_queue&>(*default_queue);
		expectations.push_back(
			NAMED_REQUIRE_CALL(queue, submit(ANY(const command&)))
				.LR_WITH(_1.get_program() == program)
				.WITH(_1.get_outputs().size() == 1)
				.WITH(_1.get_outputs()[0] == output_buffer)
				.WITH(_1.get_inputs().size() == 1)
				.WITH(_1.get_inputs()[0] == input_storage)
				.WITH(
					scratch_buffer
						? (_1.get_scratch().size() == 1 &&
						   _1.get_scratch()[0] == scratch_buffer)
						: _1.get_scratch().empty()
				)
		);
	}

	std::shared_ptr<mock_device> device;
	std::shared_ptr<memory_allocator> host_allocator;
	std::shared_ptr<mock_memory_allocator> device_allocator;
	mock_memory_resource host_resource;
	mock_memory_resource device_resource;
	std::shared_ptr<command_queue> default_queue;
	std::shared_ptr<const device_session> session;
	device_context context;
	std::shared_ptr<program_manager> manager;
	std::shared_ptr<dispatcher> eager_dispatcher;
	std::shared_ptr<mock_program> program;

	mock_operation op;
	mock_operation_shape_policy shape_policy;
	mock_operation_data_type_policy dtype_policy;

	// Keep mock buffers alive, and hold the named expectations so that they are
	// torn down (unlinked from their mocks) before the mocks they reference.
	std::vector<std::shared_ptr<mock_buffer>> buffers;
	std::vector<std::unique_ptr<trompeloeil::expectation>> expectations;
};

} // namespace


TEST_CASE(
	"eager_dispatcher dispatch throws when the device context has no "
	"active queue",
	"[eager_dispatcher]"
)
{
	const auto dispatcher = make_eager_dispatcher(
		std::make_shared<program_manager>()
	);

	mock_operation op;
	array output;
	const const_array_ref input;

	// An empty device context exposes a null active queue.
	const device_context empty_context;

	CHECK_THROWS_AS(
		dispatcher->dispatch(
			op,
			make_span(&output, 1),
			make_span(&input, 0),
			empty_context
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch throws when the operand counts do not "
	"match the operation arity",
	"[eager_dispatcher]"
)
{
	REQUIRE_CALL(op, get_arity())
		.RETURN(operation_arity::unary()); // Expects one output and one input.

	array output;
	const const_array_ref input;

	// One output but no inputs: violates the unary arity.
	CHECK_THROWS_AS(
		eager_dispatcher->dispatch(
			op,
			make_span(&output, 1),
			make_span(&input, 0),
			context
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch throws when an input operand lacks "
	"storage",
	"[eager_dispatcher]"
)
{
	expect_unary_operation(shape_type{4}, numerical_type::float32);

	array output;           // Fresh output, storage to be resolved.
	const const_array_ref input; // Input without any backing storage.

	CHECK_THROWS_AS(
		eager_dispatcher->dispatch(
			op,
			make_span(&output, 1),
			make_span(&input, 1),
			context
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch accepts a pre-allocated output whose "
	"shape or data type differs from the deduced descriptor",
	"[eager_dispatcher]"
)
{
	// The operation deduces a contiguous {4} float32 output. A pre-allocated
	// output that differs in shape or data type is forwarded to the policies'
	// accept(), which decide its admissibility; here they admit it.
	shape_type user_shape;
	numerical_type user_type;
	std::tie(user_shape, user_type) = GENERATE(
		table<shape_type, numerical_type>({
			{ shape_type{2}, numerical_type::float32 }, // Different shape.
			{ shape_type{4}, numerical_type::float64 }, // Different data type.
		})
	);

	expect_unary_operation(shape_type{4}, numerical_type::float32);

	// The policies are queried with the user-supplied descriptor alongside the
	// canonical one deduced above, and accept it.
	expectations.push_back(
		NAMED_REQUIRE_CALL(
			shape_policy,
			accept(
				ANY(const operation_descriptor&),
				ANY(span<const shape_type>),
				ANY(span<const shape_type>),
				ANY(span<const shape_type>)
			)
		)
			.WITH(_2.size() == 1 && _2[0] == user_shape)       // User output.
			.WITH(_3.size() == 1 && _3[0] == shape_type{4})    // Canonical.
	);
	expectations.push_back(
		NAMED_REQUIRE_CALL(
			dtype_policy,
			accept(
				ANY(const operation_descriptor&),
				ANY(span<const numerical_type>),
				ANY(span<const numerical_type>),
				ANY(span<const numerical_type>)
			)
		)
			.WITH(_2.size() == 1 && _2[0] == user_type)               // User.
			.WITH(_3.size() == 1 && _3[0] == numerical_type::float32) // Canonical.
	);

	register_program_builder();
	expect_program_scratch(); // No scratch required.

	// The pre-allocated output already owns storage, so no allocation happens:
	// the existing buffer is reused as-is and the command is submitted normally.
	const auto output_buffer = make_buffer();
	array output(output_buffer, make_descriptor(user_shape, user_type));

	const const_array input_owner =
		make_input_with_storage(shape_type{4}, numerical_type::float32);
	const const_array_ref input = input_owner;
	expect_command_submission(output_buffer, input.share_storage());

	CHECK_NOTHROW(
		eager_dispatcher->dispatch(
			op,
			make_span(&output, 1),
			make_span(&input, 1),
			context
		)
	);

	// The pre-allocated storage is preserved.
	CHECK( output.get_storage() == output_buffer.get() );
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch rejects an undetermined output data type "
	"when nothing is pre-allocated to settle it",
	"[eager_dispatcher]"
)
{
	// A rule whose output type is free leaves it unknown for a user supplied
	// output to settle. With no output pre-allocated there is nothing to
	// settle it, and sizing a buffer from an unknown type would allocate
	// whatever get_size() happens to return, so this must be diagnosed
	// instead. No allocation and no program build may take place.
	expect_unary_operation(shape_type{4}, numerical_type::unknown);

	array output; // No storage, so nothing determines the data type.

	const const_array input_owner =
		make_input_with_storage(shape_type{4}, numerical_type::float32);
	const const_array_ref input = input_owner;

	CHECK_THROWS_AS(
		eager_dispatcher->dispatch(
			op,
			make_span(&output, 1),
			make_span(&input, 1),
			context
		),
		std::invalid_argument
	);

	CHECK( output.get_storage() == nullptr );
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch builds the program, allocates output "
	"storage and submits the command to the active queue",
	"[eager_dispatcher]"
)
{
	shape_type out_shape;
	numerical_type out_type;
	std::tie(out_shape, out_type) = GENERATE(
		table<shape_type, numerical_type>({
			{ shape_type{4}, numerical_type::float32 },
			{ shape_type{2, 3}, numerical_type::float64 },
			{ shape_type{8}, numerical_type::int16 },
		})
	);

	expect_unary_operation(out_shape, out_type);
	register_program_builder();
	expect_program_scratch(); // No scratch required.

	// Output storage is allocated from the device allocator, sized to fit the
	// deduced descriptor.
	const auto expected_size =
		compute_storage_requirement(make_descriptor(out_shape, out_type));
	const auto output_buffer = make_buffer();
	REQUIRE_CALL(
		*device_allocator,
		allocate(expected_size, ANY(std::size_t), ANY(command_queue*))
	)
		.RETURN(output_buffer);

	const const_array input_owner = make_input_with_storage(out_shape, out_type);
	const const_array_ref input = input_owner;
	expect_command_submission(output_buffer, input.share_storage());

	array output;
	eager_dispatcher->dispatch(
		op,
		make_span(&output, 1),
		make_span(&input, 1),
		context
	);

	// The freshly allocated storage is written back into the output operand.
	CHECK( output.get_storage() == output_buffer.get() );
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch reuses a matching pre-allocated output "
	"storage instead of allocating a new one",
	"[eager_dispatcher]"
)
{
	shape_type out_shape;
	numerical_type out_type;
	std::tie(out_shape, out_type) = GENERATE(
		table<shape_type, numerical_type>({
			{ shape_type{4}, numerical_type::float32 },
			{ shape_type{2, 3}, numerical_type::float64 },
			{ shape_type{8}, numerical_type::int16 },
		})
	);

	expect_unary_operation(out_shape, out_type);
	expect_policies_accept();
	register_program_builder();
	expect_program_scratch(); // No scratch required.

	// The output already owns storage with a matching descriptor. No allocate
	// expectation is set up: any call to the allocator would fail the test,
	// proving the existing storage is reused.
	const auto output_buffer = make_buffer();
	array output(output_buffer, make_descriptor(out_shape, out_type));

	const const_array input_owner = make_input_with_storage(out_shape, out_type);
	const const_array_ref input = input_owner;
	expect_command_submission(output_buffer, input.share_storage());

	eager_dispatcher->dispatch(
		op,
		make_span(&output, 1),
		make_span(&input, 1),
		context
	);

	// The pre-allocated storage is preserved.
	CHECK( output.get_storage() == output_buffer.get() );
}

TEST_CASE_METHOD(
	eager_dispatcher_fixture,
	"eager_dispatcher dispatch allocates the scratch buffers required "
	"by the program and binds them to the submitted command",
	"[eager_dispatcher]"
)
{
	std::size_t scratch_size;
	std::size_t scratch_alignment;
	std::tie(scratch_size, scratch_alignment) = GENERATE(
		table<std::size_t, std::size_t>({
			{ 256, 16 },
			{ 1024, 64 },
			{ 64, 32 },
		})
	);

	expect_unary_operation(shape_type{4}, numerical_type::float32);
	register_program_builder();

	const program_scratch_requirement scratch_requirement(
		scratch_size,
		scratch_alignment,
		memory_resource_affinity::device
	);
	expect_program_scratch(make_span(&scratch_requirement, 1));

	// The output (16 bytes) and the scratch buffers are both allocated from the
	// device allocator, distinguished here by their size.
	const auto output_size =
		compute_storage_requirement(make_descriptor(shape_type{4}, numerical_type::float32));
	REQUIRE( output_size != scratch_size );

	const auto output_buffer = make_buffer();
	REQUIRE_CALL(
		*device_allocator,
		allocate(output_size, ANY(std::size_t), ANY(command_queue*))
	)
		.RETURN(output_buffer);

	const auto scratch_buffer = make_buffer();
	REQUIRE_CALL(
		*device_allocator,
		allocate(scratch_size, scratch_alignment, ANY(command_queue*))
	)
		.RETURN(scratch_buffer);

	const const_array input_owner =
		make_input_with_storage(shape_type{4}, numerical_type::float32);
	const const_array_ref input = input_owner;
	expect_command_submission(
		output_buffer,
		input.share_storage(),
		scratch_buffer
	);

	array output;
	eager_dispatcher->dispatch(
		op,
		make_span(&output, 1),
		make_span(&input, 1),
		context
	);
}

TEST_CASE(
	"make_eager_dispatcher builds a non-null dispatcher",
	"[eager_dispatcher]"
)
{
	const auto command_manager = std::make_shared<program_manager>();

	const auto dispatcher = make_eager_dispatcher(command_manager);

	CHECK( dispatcher != nullptr );
}

TEST_CASE(
	"make_eager_dispatcher throws when the command manager is null",
	"[eager_dispatcher]"
)
{
	CHECK_THROWS_AS(
		make_eager_dispatcher(
			std::shared_ptr<const program_manager>()
		),
		std::invalid_argument
	);
}
