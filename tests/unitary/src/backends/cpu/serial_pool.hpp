// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/thread_pool.hpp>

#include <memory>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A pool of no workers, shared by every case that does not care.
 *
 * Keeps every loop on the calling thread, which is what most cases want: they
 * are about what a program computes, not about how many threads it computes
 * it on. Spawning none also keeps these cases off the one thing this backend
 * does that a case cannot observe, which is starting and joining threads.
 *
 * Held by shared_ptr because a @ref command_queue shares ownership of the
 * pool it runs programs over.
 *
 * @return const std::shared_ptr<thread_pool>& The pool.
 */
inline const std::shared_ptr<thread_pool>& get_serial_pool()
{
	static const auto instance = std::make_shared<thread_pool>(0);
	return instance;
}

} // namespace cpu
} // namespace rexlib
