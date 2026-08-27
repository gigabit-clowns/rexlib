// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device.hpp>

#include <memory>

namespace rexlib
{
namespace cpu
{

class thread_pool;

/**
 * @brief Implementation of the @ref device interface to represent the CPU.
 *
 * Owns the threads every queue it hands out runs its programs over. The
 * device is the longest lived thing this backend has that is not static, so
 * owning them here is what keeps the workers joined while the program is
 * still running: a pool held by a static instead would be torn down during
 * static destruction, which on Windows means joining threads the loader has
 * already stopped.
 */
class REXLIB_API device final
	: public rexlib::device
{
public:
	/**
	 * @brief Construct a device over its own threads.
	 *
	 * @param pool The threads to run programs over. Built with
	 * @ref thread_pool::get_default_worker_count workers when null, which is
	 * what every caller but a test wants.
	 */
	explicit device(std::shared_ptr<thread_pool> pool = nullptr);
	~device() override;

	const memory_resource& 
	get_memory_resource(memory_resource_affinity affinity) const override;

	std::shared_ptr<rexlib::command_queue> create_command_queue() const override;

	std::shared_ptr<rexlib::event>
	create_event(event_usage_flags usage) const override;

	/**
	 * @brief Get the threads this device runs its programs over.
	 *
	 * @return const std::shared_ptr<thread_pool>& The pool.
	 */
	const std::shared_ptr<thread_pool>& get_thread_pool() const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<thread_pool> m_pool;
};

} // namespace cpu
} // namespace rexlib
