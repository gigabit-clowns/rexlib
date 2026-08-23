// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/execution_context.hpp>
#include <rex/core/dispatch/operation.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/span.hpp>
#include <rex/core/hardware/buffer.hpp>
#include <rex/core/hardware/command_queue.hpp>
#include <rex/core/hardware/device_context.hpp>
#include <rex/core/hardware/device_properties.hpp>
#include <rex/core/hardware/device_session.hpp>
#include <rex/core/hardware/memory_allocator.hpp>
#include <rex/core/hardware/memory_resource_affinity.hpp>

#include "../../core/dispatch/mock/mock_dispatcher.hpp"
#include "../../core/hardware/mock/mock_buffer.hpp"
#include "../../core/hardware/mock/mock_command_queue.hpp"
#include "../../core/hardware/mock/mock_device.hpp"
#include "../../core/hardware/mock/mock_memory_allocator.hpp"
#include "../../core/hardware/mock/mock_memory_resource.hpp"

#include <trompeloeil.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <typeinfo>
#include <vector>

namespace rex
{
namespace test
{

/**
 * @brief What a mocked dispatch() call saw, kept for later inspection.
 *
 * The verbs build their operation as a temporary and hand it straight to
 * the dispatcher, so nothing about the call survives the call itself unless
 * it is copied out here.
 */
struct dispatch_record
{
	bool called = false;
	const std::type_info *operation_type = nullptr;
	std::size_t num_outputs = 0;
	std::size_t num_inputs = 0;
	std::vector<const buffer*> output_storages;
	std::vector<const buffer*> input_storages;
	const device_session *session = nullptr;

	/**
	 * @brief Hook invoked while the operation is still alive.
	 *
	 * An operation carrying parameters, such as a fill value or a set of
	 * reduction axes, is only reachable during the call: the verb passes a
	 * temporary, so a pointer kept afterwards would dangle. Set this to
	 * copy out whatever a particular test needs.
	 */
	std::function<void(const operation&)> inspect;

	void operator()(
		const operation &op,
		span<array> outputs,
		span<const const_array_ref> inputs,
		const device_context &device_context
	)
	{
		called = true;
		operation_type = &typeid(op);
		num_outputs = outputs.size();
		num_inputs = inputs.size();

		output_storages.clear();
		for (const auto &output : outputs)
		{
			output_storages.push_back(output.get_storage());
		}

		input_storages.clear();
		for (const auto &input : inputs)
		{
			input_storages.push_back(input.get_storage());
		}

		session = device_context.get_device_session().get();

		if (inspect)
		{
			inspect(op);
		}
	}

	/**
	 * @brief Get the storage of an output operand.
	 *
	 * @param index Index of the operand.
	 * @return const buffer* The storage, or null when the operand does not
	 * exist or carried none. An absent operand is not fatal, so that a
	 * failing check reports the mismatch rather than crashing the run.
	 */
	const buffer* get_output_storage(std::size_t index) const noexcept
	{
		return index < output_storages.size()
			? output_storages[index]
			: nullptr;
	}

	/**
	 * @brief Get the storage of an input operand.
	 *
	 * @param index Index of the operand.
	 * @return const buffer* The storage, or null when the operand does not
	 * exist or carried none.
	 */
	const buffer* get_input_storage(std::size_t index) const noexcept
	{
		return index < input_storages.size()
			? input_storages[index]
			: nullptr;
	}
};

/**
 * @brief Execution context wired to mocks, for testing functional verbs.
 *
 * Standing a mock dispatcher in for the backend is what lets a verb be
 * tested without any program builder implementing its operation. A verb is
 * responsible for building the right operation and handing it the right
 * operands; whether a backend can run it is a separate question, answered
 * by the integration tests.
 */
class verb_dispatch_fixture
{
public:
	verb_dispatch_fixture()
		: device(std::make_shared<mock_device>())
		, host_allocator(std::make_shared<mock_memory_allocator>())
		, device_allocator(std::make_shared<mock_memory_allocator>())
		, default_queue(std::make_shared<mock_command_queue>())
		, dispatcher(std::make_shared<mock_dispatcher>())
	{
		device_properties properties;
		properties.set_optimal_data_alignment(128);

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

		session = std::make_shared<device_session>(
			device,
			std::move(properties)
		);

		context = execution_context(
			device_context(session),
			dispatcher
		);
	}

protected:
	/// A verb producing one array from one.
	using unary_verb = array (*)(
		const_array_ref,
		const execution_context&,
		array*
	);

	/// A verb producing one array from two.
	using binary_verb = array (*)(
		const_array_ref,
		const_array_ref,
		const execution_context&,
		array*
	);

	/// A verb producing one array from three.
	using ternary_verb = array (*)(
		const_array_ref,
		const_array_ref,
		const_array_ref,
		const execution_context&,
		array*
	);

	/// A verb reducing one array over a set of axes.
	using reduction_verb = array (*)(
		const_array_ref,
		span<const std::ptrdiff_t>,
		bool,
		const execution_context&,
		array*
	);

	/// A verb reducing one array over every axis.
	using full_reduction_verb = array (*)(
		const_array_ref,
		const execution_context&,
		array*
	);

	array_descriptor make_descriptor(
		std::vector<std::size_t> extents = { 2, 3 },
		numerical_type data_type = numerical_type::float32
	) const
	{
		return array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			data_type
		);
	}

	/// Create an array backed by a fresh mock buffer.
	array make_operand(
		std::shared_ptr<mock_buffer> &storage,
		numerical_type data_type = numerical_type::float32
	) const
	{
		storage = std::make_shared<mock_buffer>();
		return array(storage, make_descriptor({ 2, 3 }, data_type));
	}

	/**
	 * @brief Check that a unary verb dispatches @p Op.
	 *
	 * Covers the two cases every single output verb shares: the output left
	 * to be deduced, and the storage of a pre-allocated output reused.
	 *
	 * @tparam Op The operation the verb is expected to build.
	 * @param verb The verb under test.
	 * @param data_type Element type of the operand, for verbs whose rule
	 * does not admit the default.
	 */
	template <typename Op>
	void check_unary_verb(
		unary_verb verb,
		numerical_type data_type = numerical_type::float32
	)
	{
		{
			std::shared_ptr<mock_buffer> x_storage;
			const array x_array = make_operand(x_storage, data_type);
			const const_array_ref x = x_array;

			dispatch_record record;
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

			verb(x, context, nullptr);

			INFO( "output deduced" );
			check_record<Op>(record, 1);
			CHECK( record.get_output_storage(0) == nullptr );
			CHECK( record.get_input_storage(0) == x_storage.get() );
		}

		{
			std::shared_ptr<mock_buffer> x_storage;
			std::shared_ptr<mock_buffer> out_storage;
			const array x_array = make_operand(x_storage, data_type);
			array out = make_operand(out_storage, data_type);
			const const_array_ref x = x_array;

			dispatch_record record;
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

			const auto result = verb(x, context, &out);

			INFO( "output storage reused" );
			check_record<Op>(record, 1);
			CHECK( record.get_output_storage(0) == out_storage.get() );
			CHECK( result.get_storage() == out_storage.get() );
		}
	}

	/**
	 * @brief Check that a binary verb dispatches @p Op.
	 *
	 * @tparam Op The operation the verb is expected to build.
	 * @param verb The verb under test.
	 * @param data_type Element type of the operands.
	 */
	template <typename Op>
	void check_binary_verb(
		binary_verb verb,
		numerical_type data_type = numerical_type::float32
	)
	{
		{
			std::shared_ptr<mock_buffer> x_storage;
			std::shared_ptr<mock_buffer> y_storage;
			const array x_array = make_operand(x_storage, data_type);
			const array y_array = make_operand(y_storage, data_type);
			const const_array_ref x = x_array;
			const const_array_ref y = y_array;

			dispatch_record record;
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

			verb(x, y, context, nullptr);

			INFO( "output deduced" );
			check_record<Op>(record, 2);
			CHECK( record.get_output_storage(0) == nullptr );
			CHECK( record.get_input_storage(0) == x_storage.get() );

			// The operands must reach the dispatcher in the order the verb
			// was given them, which is the whole of what a binary verb has
			// to get right beyond naming its operation.
			CHECK( record.get_input_storage(1) == y_storage.get() );
		}

		{
			std::shared_ptr<mock_buffer> x_storage;
			std::shared_ptr<mock_buffer> y_storage;
			std::shared_ptr<mock_buffer> out_storage;
			const array x_array = make_operand(x_storage, data_type);
			const array y_array = make_operand(y_storage, data_type);
			array out = make_operand(out_storage, data_type);
			const const_array_ref x = x_array;
			const const_array_ref y = y_array;

			dispatch_record record;
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

			const auto result = verb(x, y, context, &out);

			INFO( "output storage reused" );
			check_record<Op>(record, 2);
			CHECK( record.get_output_storage(0) == out_storage.get() );
			CHECK( result.get_storage() == out_storage.get() );
		}
	}

	/**
	 * @brief Check that a ternary verb dispatches @p Op.
	 *
	 * @tparam Op The operation the verb is expected to build.
	 * @param verb The verb under test.
	 * @param first_type Element type of the first operand, which a
	 * selection needs to differ from the rest.
	 * @param data_type Element type of the remaining operands.
	 */
	template <typename Op>
	void check_ternary_verb(
		ternary_verb verb,
		numerical_type first_type = numerical_type::float32,
		numerical_type data_type = numerical_type::float32
	)
	{
		std::shared_ptr<mock_buffer> x_storage;
		std::shared_ptr<mock_buffer> y_storage;
		std::shared_ptr<mock_buffer> z_storage;
		const array x_array = make_operand(x_storage, first_type);
		const array y_array = make_operand(y_storage, data_type);
		const array z_array = make_operand(z_storage, data_type);
		const const_array_ref x = x_array;
		const const_array_ref y = y_array;
		const const_array_ref z = z_array;

		dispatch_record record;
		REQUIRE_CALL(*dispatcher, dispatch(
			trompeloeil::_, trompeloeil::_,
			trompeloeil::_, trompeloeil::_
		))
			.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

		verb(x, y, z, context, nullptr);

		check_record<Op>(record, 3);
		CHECK( record.get_output_storage(0) == nullptr );
		CHECK( record.get_input_storage(0) == x_storage.get() );
		CHECK( record.get_input_storage(1) == y_storage.get() );
		CHECK( record.get_input_storage(2) == z_storage.get() );
	}

	/**
	 * @brief Check that a reduction verb dispatches @p Op.
	 *
	 * Covers both spellings: the axes given explicitly, and the overload
	 * without them, which reduces over every axis. The axes an operation
	 * ends up carrying are read back off it, which is what shows that the
	 * verb resolved them rather than deferring the question.
	 *
	 * @tparam Op The operation the verb is expected to build.
	 * @param over_axes The verb taking a set of axes.
	 * @param over_everything The verb reducing over every axis.
	 * @param data_type Element type of the operand.
	 */
	template <typename Op>
	void check_reduction_verb(
		reduction_verb over_axes,
		full_reduction_verb over_everything,
		numerical_type data_type = numerical_type::float32
	)
	{
		{
			std::shared_ptr<mock_buffer> x_storage;
			const array x_array = make_operand(x_storage, data_type);
			const const_array_ref x = x_array;

			// Referred from the end, so that resolving it is observable.
			const std::vector<std::ptrdiff_t> axes = { -1 };

			dispatch_record record;
			std::vector<std::size_t> seen;
			record.inspect = [&seen] (const operation &op)
			{
				const auto &typed = dynamic_cast<const Op&>(op);
				const auto resolved = typed.get_shape_policy().get_axes();
				seen.assign(resolved.begin(), resolved.end());
			};
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

			over_axes(x, make_span(axes), false, context, nullptr);

			INFO( "reduced over the last axis" );
			check_record<Op>(record, 1);
			CHECK( record.get_input_storage(0) == x_storage.get() );

			// The operand is two dimensional, so the last axis is axis 1.
			// An operation holds explicit axes, never a request to work
			// one out later.
			REQUIRE( seen.size() == 1 );
			CHECK( seen[0] == 1 );
		}

		{
			std::shared_ptr<mock_buffer> x_storage;
			const array x_array = make_operand(x_storage, data_type);
			const const_array_ref x = x_array;

			dispatch_record record;
			std::vector<std::size_t> seen;
			record.inspect = [&seen] (const operation &op)
			{
				const auto &typed = dynamic_cast<const Op&>(op);
				const auto resolved = typed.get_shape_policy().get_axes();
				seen.assign(resolved.begin(), resolved.end());
			};
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

			over_everything(x, context, nullptr);

			INFO( "reduced over every axis" );
			check_record<Op>(record, 1);

			// Reducing over all of them is spelled out into a list rather
			// than carried into the operation as a mode.
			REQUIRE( seen.size() == 2 );
			CHECK( seen[0] == 0 );
			CHECK( seen[1] == 1 );
		}
	}

	/// Assert the parts of a record every single output verb shares.
	template <typename Op>
	void check_record(
		const dispatch_record &record,
		std::size_t expected_inputs
	) const
	{
		CHECK( record.called );
		REQUIRE( record.operation_type != nullptr );
		CHECK( *record.operation_type == typeid(Op) );
		CHECK( record.num_outputs == 1 );
		CHECK( record.num_inputs == expected_inputs );
		CHECK( record.session == session.get() );
	}

	std::shared_ptr<mock_device> device;
	std::shared_ptr<mock_memory_allocator> host_allocator;
	std::shared_ptr<mock_memory_allocator> device_allocator;
	mock_memory_resource host_resource;
	mock_memory_resource device_resource;
	std::shared_ptr<command_queue> default_queue;
	std::shared_ptr<const device_session> session;
	std::shared_ptr<mock_dispatcher> dispatcher;
	execution_context context;
};

} // namespace test
} // namespace rex
