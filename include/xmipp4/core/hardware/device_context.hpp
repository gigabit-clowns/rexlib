// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "memory_resource_affinity.hpp"
#include "../platform/dynamic_shared_object.h"

#include <array>
#include <cstddef>
#include <memory>

namespace rex
{

class device_session;
class command_queue;
class memory_allocator;

/**
 * @brief Scoped, configurable view over a @ref device_session.
 *
 * A @c device_context layers the per-use, configurable state on top of the
 * immutable @ref device_session: the @em active @ref command_queue that work
 * is submitted to, and the @ref memory_allocator selected for each
 * @ref memory_resource_affinity. 
 *
 * The context is a lightweight, copyable value with functional-update
 * semantics: @ref on_queue and @ref with_allocator do not mutate the receiver,
 * they return a modified copy. This makes a context safe to share (including
 * across threads) while still allowing callers to derive scoped variants, e.g.
 * to bind a special-purpose allocator for a single operation without disturbing
 * any other user of the original context. Copies are cheap: the session, the
 * queue and the allocators are all shared pointers.
 *
 * @par Allocator slots
 * For a context wrapping an session, every affinity slot always holds the
 * allocator that @ref get_allocator would return: at construction the slots are
 * seeded from the @ref device_session allocators, and @ref with_allocator
 * either installs an override or, when given a null allocator, reverts the slot
 * back to the session allocator.
 *
 * @par Empty state
 * A default-constructed or moved-from context is @em empty: it holds no
 * session, no queue and no allocators. Querying it is well defined; the
 * accessors simply return null shared pointers. A populated context (one
 * constructed from a non-null session) never returns null from any accessor.
 */
class device_context
{
public:
	/**
	 * @brief Construct an empty context holding no session.
	 *
	 * Every accessor of the resulting context returns null until it is
	 * replaced by assignment.
	 */
	device_context() noexcept = default;

	/**
	 * @brief Construct a context for @p session.
	 *
	 * The active queue is set to the default queue of the wrapped device and
	 * every allocator slot is seeded with the corresponding @p session
	 * allocator.
	 *
	 * @param session Device session to wrap. Must not be null.
	 *
	 * @throws std::invalid_argument if @p session is null.
	 */
	REXLIB_API
	explicit device_context(std::shared_ptr<const device_session> session);

	device_context(const device_context &other) = default;
	device_context(device_context &&other) noexcept = default;
	~device_context() = default;

	device_context& operator=(const device_context &other) = default;
	device_context& operator=(device_context &&other) noexcept = default;

	/**
	 * @brief Retrieve the wrapped device session.
	 *
	 * @return A reference to the wrapped session, or to a null pointer if
	 * this context is empty.
	 */
	REXLIB_API
	const std::shared_ptr<const device_session>&
	get_device_session() const noexcept;

	/**
	 * @brief Retrieve the active command queue.
	 *
	 * Work produced through this context is submitted onto this queue.
	 *
	 * @return A reference to the active queue, or to a null pointer if this
	 * context is empty.
	 */
	REXLIB_API
	const std::shared_ptr<command_queue>& get_active_queue() const noexcept;

	/**
	 * @brief Retrieve the allocator for the given affinity.
	 *
	 * Returns the override installed through @ref with_allocator, or the
	 * @ref device_session allocator when no override is in effect.
	 *
	 * @param affinity The desired memory_resource_affinity (host or device).
	 * @return A reference to the allocator for @p affinity, or to a null
	 * pointer if this context is empty.
	 */
	REXLIB_API
	const std::shared_ptr<memory_allocator>&
	get_allocator(memory_resource_affinity affinity) const noexcept;

	/**
	 * @brief Derive a context that submits onto a different queue.
	 *
	 * @param queue The active queue of the returned context. The provided queue
	 * must have been provided by the device held by this context. Passing null
	 * uses device_session's default queue on the returned device_context.
	 * @return A copy of this context with @p queue as its active queue.
	 */
	REXLIB_API
	device_context on_queue(std::shared_ptr<command_queue> queue) const;

	/**
	 * @brief Derive a context with a different allocator for an affinity.
	 *
	 * Installs @p allocator as the allocator for @p affinity. Passing a null
	 * @p allocator reverts the slot to the @ref device_session allocator for
	 * @p affinity (or to null when this context is empty).
	 *
	 * The override must produce buffers reachable by the role implied by the
	 * slot: an allocator bound to @ref memory_resource_affinity::host must be
	 * host-accessible, and one bound to @ref memory_resource_affinity::device
	 * must be device-accessible. 
	 *
	 * @param affinity The affinity slot to update.
	 * @param allocator The override to install, or null to revert to the
	 * session allocator. Must satisfy the allocator requirements for the
	 * slot where it is being installed.
	 * @return A copy of this context with the updated allocator slot.
	 */
	REXLIB_API
	device_context with_allocator(
		memory_resource_affinity affinity,
		std::shared_ptr<memory_allocator> allocator
	) const;

private:
	using allocator_array_type = std::array<
		std::shared_ptr<memory_allocator>,
		static_cast<std::size_t>(memory_resource_affinity::count)
	>;

	std::shared_ptr<const device_session> m_session;
	std::shared_ptr<command_queue> m_active_queue;
	allocator_array_type m_allocators;
};

} // namespace rex
