// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "dispatcher.hpp"

#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/core/hardware/device_context.hpp>

#include <memory>

namespace xmipp4
{


/**
 * @brief Pairs the hardware resources and the dispatch strategy used to run
 * operations.
 *
 * An @c execution_context bundles the two pieces of state that every
 * operation needs in order to run: a
 * @ref device_context describing @em where the work executes (the
 * device, its active @ref command_queue and its
 * @ref memory_allocator slots), and a @ref dispatcher
 * describing @em how operations are turned into device work.
 *
 * Like @ref device_context, the execution context is a lightweight,
 * copyable value with functional-update semantics: @ref with_device_context
 * and @ref with_dispatcher do not mutate the receiver, they return a
 * modified copy. This makes an execution context cheap to share (the device
 * context and the dispatcher are both held by shared pointer) while letting
 * callers derive scoped variants, e.g. to run a single operation on a
 * different queue without disturbing any other user of the original
 * execution context.
 *
 * @par Empty state
 * A default-constructed or moved-from execution context is @em empty: its
 * device context is empty (see @ref device_context) and its
 * dispatcher is null. Querying it is well defined; the accessors simply
 * return an empty device context and a null dispatcher.
 */
class execution_context
{
public:
	/**
	 * @brief Construct an empty execution context.
	 *
	 * The resulting execution context holds an empty
	 * @ref device_context and a null dispatcher.
	 */
	REXLIB_API
	execution_context() noexcept;

	/**
	 * @brief Construct an execution context with an explicit dispatcher.
	 *
	 * @param device_context The hardware resources on which operations execute.
	 * @param dispatcher The dispatcher used to turn operations into device
	 * work.
	 */
	REXLIB_API
	execution_context(
		device_context device_context,
		std::shared_ptr<dispatcher> dispatcher
	);

	REXLIB_API
	execution_context(const execution_context &other);
	REXLIB_API
	execution_context(execution_context &&other) noexcept;
	REXLIB_API
	~execution_context();

	REXLIB_API
	execution_context& operator=(const execution_context &other);
	REXLIB_API
	execution_context& operator=(execution_context &&other) noexcept;

	/**
	 * @brief Retrieve the device context describing where operations execute.
	 *
	 * @return A reference to the wrapped device context.
	 */
	REXLIB_API
	const device_context&
	get_device_context() const noexcept;

	/**
	 * @brief Retrieve the dispatcher used to execute operations.
	 *
	 * @return A reference to the dispatcher, or to a null pointer when this
	 * execution context is empty.
	 */
	REXLIB_API
	const std::shared_ptr<dispatcher>&
	get_dispatcher() const noexcept;

	/**
	 * @brief Derive an execution context with a different device context.
	 *
	 * Replaces the hardware resources describing @em where operations execute
	 * while preserving the dispatcher. Per-use derivations such as switching
	 * queue or overriding an allocator are expressed on the
	 * @ref device_context itself, e.g.
	 * `ctx.with_device_context(ctx.get_device_context().on_queue(queue));`
	 *
	 * @param device_context The device context of the returned execution
	 * context.
	 * @return A copy of this execution context with @p device_context as its
	 * device context and the same dispatcher.
	 */
	REXLIB_API
	execution_context with_device_context(
		device_context device_context
	) const;

	/**
	 * @brief Derive an execution context with a different dispatcher.
	 *
	 * @param dispatcher The dispatcher of the returned execution context.
	 * @return A copy of this execution context with @p dispatcher as its
	 * dispatcher and the same device context.
	 */
	REXLIB_API
	execution_context with_dispatcher(
		std::shared_ptr<dispatcher> dispatcher
	) const;

private:
	device_context m_device_context;
	std::shared_ptr<dispatcher> m_dispatcher;
};

} // namespace xmipp4
