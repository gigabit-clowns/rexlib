// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/completion_notifier.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>

namespace rexlib
{

/**
 * @brief A completion that resolves once a fixed number of units of
 * work have all reported in.
 *
 * A semaphore-style countdown from a starting count to zero, not a
 * counter compared against a separately kept target. Resolves only
 * once every unit of work has reported, regardless of which ones
 * failed or when, so a caller of @ref completion::wait never observes
 * this ready while another unit of work behind it is still running.
 * The first exception reported, if any, is what @ref get rethrows; the
 * rest are discarded.
 *
 * Implements both @ref completion and @ref completion_notifier so that
 * the same object serves both audiences, each reached through the
 * interface meant for it: the query side goes to whoever asked for the
 * work, the notifying side goes to the @ref executor running it.
 *
 * @par Thread safety
 * @ref notify_done may be called concurrently, from as many threads as
 * the starting count.
 */
class counting_completion final
	: public completion
	, public completion_notifier
{
public:
	/**
	 * @brief Construct a completion for a known number of units of work.
	 *
	 * A @p count of zero constructs a completion that is already
	 * resolved.
	 *
	 * @param count How many times @ref notify_done is reported before
	 * this resolves.
	 */
	REXLIB_API
	explicit counting_completion(std::size_t count);

	counting_completion(const counting_completion &other) = delete;
	counting_completion(counting_completion &&other) = delete;

	REXLIB_API
	~counting_completion() override;

	counting_completion&
	operator=(const counting_completion &other) = delete;
	counting_completion&
	operator=(counting_completion &&other) = delete;

	REXLIB_API
	void notify_done(std::exception_ptr error = nullptr) override;

	REXLIB_API
	void wait() const override;

	REXLIB_API
	bool is_ready() const noexcept override;

	REXLIB_API
	void get() const override;

private:
	class implementation;
	REXLIB_STD_MEMBER_INTERFACE
	std::unique_ptr<implementation> m_implementation;
};

} // namespace rexlib
