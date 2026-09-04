// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/concurrency/counting_completion.hpp>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <mutex>

namespace rexlib
{

class counting_completion::implementation
{
public:
	explicit implementation(std::size_t count)
		: m_remaining(count)
	{
	}

	void notify_done(std::exception_ptr error)
	{
		if (error)
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_first_error)
			{
				m_first_error = error;
			}
		}

		if (m_remaining.fetch_sub(1, std::memory_order_acq_rel) == 1)
		{
			// Empty on purpose: synchronizes with wait()'s check-then-block
			// on the same mutex, so a waiter cannot be caught between
			// checking the predicate and registering for notify_all below.
			// Removing this lock/unlock reintroduces that lost-wakeup race.
			{
				const std::lock_guard<std::mutex> lock(m_mutex);
			}
			m_cv.notify_all();
		}
	}

	void wait() const
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_cv.wait(lock, [this] { return is_ready(); });
	}

	bool is_ready() const noexcept
	{
		return m_remaining.load(std::memory_order_acquire) == 0;
	}

	void get() const
	{
		wait();

		const std::lock_guard<std::mutex> lock(m_mutex);
		if (m_first_error)
		{
			std::rethrow_exception(m_first_error);
		}
	}

private:
	std::atomic<std::size_t> m_remaining;
	mutable std::mutex m_mutex;
	mutable std::condition_variable m_cv;
	std::exception_ptr m_first_error;
};

counting_completion::counting_completion(std::size_t count)
	: m_implementation(std::make_unique<implementation>(count))
{
}

counting_completion::~counting_completion() = default;

void counting_completion::notify_done(std::exception_ptr error)
{
	m_implementation->notify_done(error);
}

void counting_completion::wait() const
{
	m_implementation->wait();
}

bool counting_completion::is_ready() const noexcept
{
	return m_implementation->is_ready();
}

void counting_completion::get() const
{
	m_implementation->get();
}

} // namespace rexlib
