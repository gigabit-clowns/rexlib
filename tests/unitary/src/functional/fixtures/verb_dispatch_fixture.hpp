// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/operation/operation.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_properties.hpp>
#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>

#include "../../core/dispatch/mock/mock_dispatcher.hpp"
#include "../../core/hardware/mock/mock_buffer.hpp"
#include "../../core/hardware/mock/mock_command_queue.hpp"
#include "../../core/hardware/mock/mock_device.hpp"
#include "../../core/hardware/mock/mock_memory_allocator.hpp"
#include "../../core/hardware/mock/mock_memory_resource.hpp"

#include <trompeloeil.hpp>

#include <cstddef>
#include <memory>
#include <typeinfo>
#include <vector>

namespace rexlib
{
namespace test
{

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

			INFO( "output deduced" );
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.WITH( is_operation<Op>(_1) )
				.WITH( _2.size() == 1 )
				.WITH( _3.size() == 1 )
				.LR_WITH( in_session(_4) )
				.WITH( storage_of(_2, 0) == nullptr )
				.LR_WITH( storage_of(_3, 0) == x_storage.get() );

			verb(x, context, nullptr);
		}

		{
			std::shared_ptr<mock_buffer> x_storage;
			std::shared_ptr<mock_buffer> out_storage;
			const array x_array = make_operand(x_storage, data_type);
			array out = make_operand(out_storage, data_type);
			const const_array_ref x = x_array;

			INFO( "output storage reused" );
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.WITH( is_operation<Op>(_1) )
				.WITH( _2.size() == 1 )
				.WITH( _3.size() == 1 )
				.LR_WITH( in_session(_4) )
				.LR_WITH( storage_of(_2, 0) == out_storage.get() );

			const auto result = verb(x, context, &out);

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

			INFO( "output deduced" );
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.WITH( is_operation<Op>(_1) )
				.WITH( _2.size() == 1 )
				.WITH( _3.size() == 2 )
				.LR_WITH( in_session(_4) )
				.WITH( storage_of(_2, 0) == nullptr )
				.LR_WITH( storage_of(_3, 0) == x_storage.get() )
				.LR_WITH( storage_of(_3, 1) == y_storage.get() );

			verb(x, y, context, nullptr);
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

			INFO( "output storage reused" );
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.WITH( is_operation<Op>(_1) )
				.WITH( _2.size() == 1 )
				.WITH( _3.size() == 2 )
				.LR_WITH( in_session(_4) )
				.LR_WITH( storage_of(_2, 0) == out_storage.get() );

			const auto result = verb(x, y, context, &out);

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

		REQUIRE_CALL(*dispatcher, dispatch(
			trompeloeil::_, trompeloeil::_,
			trompeloeil::_, trompeloeil::_
		))
			.WITH( is_operation<Op>(_1) )
			.WITH( _2.size() == 1 )
			.WITH( _3.size() == 3 )
			.LR_WITH( in_session(_4) )
			.WITH( storage_of(_2, 0) == nullptr )
			.LR_WITH( storage_of(_3, 0) == x_storage.get() )
			.LR_WITH( storage_of(_3, 1) == y_storage.get() )
			.LR_WITH( storage_of(_3, 2) == z_storage.get() );

		verb(x, y, z, context, nullptr);
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

			INFO( "reduced over the last axis" );
			// The operand is two dimensional, so the last axis is axis 1.
			// An operation holds explicit axes, never a request to work
			// one out later.
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.WITH( is_operation<Op>(_1) )
				.WITH( _2.size() == 1 )
				.WITH( _3.size() == 1 )
				.LR_WITH( in_session(_4) )
				.LR_WITH( storage_of(_3, 0) == x_storage.get() )
				.WITH( axes_of<Op>(_1) == std::vector<std::size_t>{ 1 } );

			over_axes(x, make_span(axes), false, context, nullptr);
		}

		{
			std::shared_ptr<mock_buffer> x_storage;
			const array x_array = make_operand(x_storage, data_type);
			const const_array_ref x = x_array;

			INFO( "reduced over every axis" );
			// Reducing over all of them is spelled out into a list rather
			// than carried into the operation as a mode.
			REQUIRE_CALL(*dispatcher, dispatch(
				trompeloeil::_, trompeloeil::_,
				trompeloeil::_, trompeloeil::_
			))
				.WITH( is_operation<Op>(_1) )
				.WITH( _2.size() == 1 )
				.WITH( _3.size() == 1 )
				.LR_WITH( in_session(_4) )
				.WITH( axes_of<Op>(_1) == std::vector<std::size_t>{ 0, 1 } );

			over_everything(x, context, nullptr);
		}
	}

	/**
	 * @brief Whether an operation is exactly an @p Op.
	 */
	template <typename Op>
	static bool is_operation(const operation &op)
	{
		return typeid(op) == typeid(Op);
	}

	/**
	 * @brief Get the storage of an output operand.
	 *
	 * @param operands The output operands of a dispatch.
	 * @param index Index of the operand.
	 * @return const buffer* The storage, or null when the operand does not
	 * exist or carried none.
	 */
	static const buffer* storage_of(span<array> operands, std::size_t index)
	{
		return index < operands.size()
			? operands[index].get_storage()
			: nullptr;
	}

	/**
	 * @brief Get the storage of an input operand.
	 *
	 * @param operands The input operands of a dispatch.
	 * @param index Index of the operand.
	 * @return const buffer* The storage, or null when the operand does not
	 * exist or carried none.
	 */
	static const buffer* storage_of(
		span<const const_array_ref> operands,
		std::size_t index
	)
	{
		return index < operands.size()
			? operands[index].get_storage()
			: nullptr;
	}

	/**
	 * @brief Get the axes a reduction-like operation carries.
	 *
	 * @tparam Op The operation, whose parameters expose @c get_axes.
	 * @return std::vector<std::size_t> The axes, or none when @p op is not an
	 * @p Op.
	 */
	template <typename Op>
	static std::vector<std::size_t> axes_of(const operation &op)
	{
		const auto *typed = dynamic_cast<const Op*>(&op);
		if (!typed)
		{
			return {};
		}

		const auto axes = typed->get_parameters().get_axes();
		return std::vector<std::size_t>(axes.begin(), axes.end());
	}

	/// Whether a dispatch runs in the session of this fixture.
	bool in_session(const device_context &device_context) const
	{
		return device_context.get_device_session().get() == session.get();
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
} // namespace rexlib
