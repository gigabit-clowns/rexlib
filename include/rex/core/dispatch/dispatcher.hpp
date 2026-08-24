// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"
#include "../span.hpp"

#include <memory>

namespace rexlib
{


class device_context;


class array;
class const_array_ref;


class operation;
class program_manager;

/**
 * @brief Abstract interface that executes operations on a set of operands.
 *
 * A dispatcher takes the immutable description of an operation together with
 * its input and output operands and turns it into work submitted to a device.
 * It owns whatever machinery is required to select and build an executable
 * program for the operation (e.g. an operation program manager and a cache of
 * backend-private resources), so that callers only need to provide the
 * operands and the hardware resources on which the work is to be carried out.
 *
 * Different implementations may realize different execution strategies, such
 * as eager submission or deferred graph building.
 */
class REXLIB_API dispatcher
{
public:
	dispatcher() noexcept;
	dispatcher(const dispatcher &other) = delete;
	dispatcher(dispatcher &&other) = delete;
	virtual ~dispatcher();

	dispatcher&
	operator=(const dispatcher &other) = delete;
	dispatcher&
	operator=(dispatcher &&other) = delete;

	/**
	 * @brief Execute an operation on the provided operands.
	 *
	 * Output operands are resolved against the operation: when they lack
	 * storage it is allocated from @p allocator, and when they already own
	 * storage it is validated and reused. The resulting command is then
	 * submitted to @p queue.
	 *
	 * @param operation The operation to be executed.
	 * @param output_operands The output operands. May be modified in place to
	 * receive freshly allocated storage and sanitized descriptors.
	 * @param input_operands The input operands. Must already own storage.
	 * @param device_context Device resources used for execution.
	 * @param queue The queue to which the operation may be executed.
	 */
	virtual void dispatch(
		const operation &operation,
		span<array> output_operands,
		span<const const_array_ref> input_operands,
		const device_context &device
	) = 0;
};

/**
 * @brief Create a dispatcher that submits operations eagerly.
 *
 * The returned dispatcher builds and submits the executable program for each
 * operation as soon as it is dispatched, caching backend-private resources
 * (compiled kernels, FFT plans, ...) so they are reused across builds with
 * matching launch configurations.
 *
 * @param program_manager The manager queried to build the program for each
 * operation. Must not be null.
 * @return A newly created eager dispatcher.
 *
 * @throws std::invalid_argument if @p program_manager is null.
 */
REXLIB_API
std::shared_ptr<dispatcher> make_eager_dispatcher(
	std::shared_ptr<const program_manager> program_manager
);

} // namespace rexlib
