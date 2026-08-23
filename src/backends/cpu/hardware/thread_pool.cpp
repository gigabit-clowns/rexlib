// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/backends/cpu/thread_pool.hpp>

#include <xmipp4/core/platform/assert.hpp>

#include <core/logger.hpp>

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace xmipp4
{
namespace cpu
{

static const char XMIPP4_NUM_THREADS_ENV_VARIABLE[] = "XMIPP4_NUM_THREADS";

namespace
{

/**
 * @brief Compute the half-open range of iterations one chunk covers.
 *
 * The first `count % chunk_count` chunks are one iteration longer than the
 * rest, which balances them without ever leaving a chunk empty.
 */
void compute_chunk_range(
	std::size_t count,
	std::size_t chunk_count,
	std::size_t chunk,
	std::size_t &begin,
	std::size_t &end
) noexcept
{
	const auto quotient = count / chunk_count;
	const auto remainder = count % chunk_count;

	begin = chunk*quotient + std::min(chunk, remainder);
	end = begin + quotient;
	if (chunk < remainder)
	{
		++end;
	}
}

} // anonymous namespace

/**
 * @brief State of a @ref thread_pool.
 *
 * The whole job description, the claim of a chunk and the accounting of the
 * chunks still running are guarded by one mutex. Claiming through an atomic
 * instead would save a lock, but a job is cut into at most one chunk per
 * participant and every chunk is at least one grain long, so the lock is taken
 * a handful of times against tens of thousands of iterations. Paying it buys
 * the guarantee that no participant can ever read half of one job and half of
 * the next.
 */
class thread_pool_implementation
{
public:
	explicit thread_pool_implementation(std::size_t worker_count);
	thread_pool_implementation(
		const thread_pool_implementation &other
	) = delete;
	~thread_pool_implementation();

	thread_pool_implementation& operator=(
		const thread_pool_implementation &other
	) = delete;

	std::size_t get_size() const noexcept;

	void run(
		std::size_t count,
		std::size_t grain_size,
		thread_pool::body_function body,
		const void *context
	);

private:
	/**
	 * @brief One chunk, snapshot out of the job description.
	 */
	struct chunk_work
	{
		thread_pool::body_function body;
		const void *context;
		std::size_t index;
		std::size_t begin;
		std::size_t end;
	};

	std::vector<std::thread> m_workers;

	std::mutex m_mutex;
	std::condition_variable m_wake; // Workers wait here for a job.
	std::condition_variable m_done; // The caller waits here for the join.

	// Held for the whole of a job, so that a second host thread submitting at
	// the same time runs its own loop inline rather than overwriting the job
	// this one published.
	std::mutex m_job_mutex;

	// The job, republished under m_mutex once per run().
	std::uint64_t m_generation;
	std::size_t m_count;
	std::size_t m_chunk_count;
	std::size_t m_next_chunk;
	std::size_t m_running;
	thread_pool::body_function m_body;
	const void *m_context;
	bool m_failed;
	std::exception_ptr m_exception;
	std::size_t m_failed_chunk;
	bool m_stop;

	void work();
	void run_chunks();
	bool claim(chunk_work &work);
	void finish_chunk(std::size_t chunk, std::exception_ptr exception);
	bool is_job_finished() const noexcept;
	void stop_and_join() noexcept;
};

thread_pool_implementation::thread_pool_implementation(
	std::size_t worker_count
)
	: m_generation(0)
	, m_count(0)
	, m_chunk_count(0)
	, m_next_chunk(0)
	, m_running(0)
	, m_body(nullptr)
	, m_context(nullptr)
	, m_failed(false)
	, m_failed_chunk(0)
	, m_stop(false)
{
	m_workers.reserve(worker_count);
	try
	{
		for (std::size_t i = 0; i < worker_count; ++i)
		{
			m_workers.emplace_back(&thread_pool_implementation::work, this);
		}
	}
	catch (...)
	{
		// Whatever was spawned before the failure has to be brought down
		// before this object stops existing.
		stop_and_join();
		throw;
	}
}

thread_pool_implementation::~thread_pool_implementation()
{
	stop_and_join();
}

std::size_t thread_pool_implementation::get_size() const noexcept
{
	return m_workers.size() + 1; // The caller participates.
}

void thread_pool_implementation::run(
	std::size_t count,
	std::size_t grain_size,
	thread_pool::body_function body,
	const void *context
)
{
	if (count == 0)
	{
		return;
	}

	if (grain_size == 0)
	{
		grain_size = 1;
	}

	const auto affordable = count / grain_size;
	const auto chunk_count = std::min(
		std::max(affordable, std::size_t(1)),
		get_size()
	);

	// Nothing to be gained from the pool: no worker, or not enough work to
	// pay for waking one.
	if (chunk_count > 1)
	{
		// The job mutex is held for the whole of a job, so failing to take it
		// means the pool is busy. try_lock rather than lock, for two reasons.
		// A second host thread submitting at the same time is served by
		// running its loop inline, which is what it did before this pool
		// existed, whereas blocking would make it slower than that. And a
		// body reaching here again finds the job it is itself part of, so
		// running inline is the only answer that does not wait on the very
		// threads that are waiting on it: this is what makes nesting safe,
		// without a caller having to know that it is nested.
		std::unique_lock<std::mutex> job_lock(m_job_mutex, std::try_to_lock);
		if (job_lock.owns_lock())
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_count = count;
				m_chunk_count = chunk_count;
				m_next_chunk = 0;
				m_running = 0;
				m_body = body;
				m_context = context;
				m_failed = false;
				m_exception = nullptr;
				m_failed_chunk = 0;
				++m_generation;
			}
			m_wake.notify_all();

			run_chunks();

			std::exception_ptr exception;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_done.wait(
					lock,
					[this] ()
					{
						return is_job_finished();
					}
				);

				exception = std::move(m_exception);
				m_exception = nullptr;
			}

			// Rethrown here, on the calling thread, so that a body throwing
			// behaves exactly as it does when the loop runs inline.
			if (exception)
			{
				std::rethrow_exception(exception);
			}

			return;
		}
	}

	body(context, 0, count);
}

void thread_pool_implementation::work()
{
	std::uint64_t seen = 0;
	for (;;)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			// Waiting on the generation rather than on a flag is what lets a
			// worker still draining the previous job pick the next one up
			// without sleeping, and what makes a spurious wake-up harmless.
			m_wake.wait(
				lock,
				[this, seen] ()
				{
					return m_stop || m_generation != seen;
				}
			);

			if (m_stop)
			{
				return;
			}

			seen = m_generation;
		}

		run_chunks();
	}
}

void thread_pool_implementation::run_chunks()
{
	chunk_work work;
	while (claim(work))
	{
		std::exception_ptr exception;
		try
		{
			work.body(work.context, work.begin, work.end);
		}
		catch (...)
		{
			exception = std::current_exception();
		}

		finish_chunk(work.index, std::move(exception));
	}
}

bool thread_pool_implementation::claim(chunk_work &work)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// A chunk that has not been claimed by the time another one has thrown is
	// abandoned: the job has no answer left to produce.
	if (m_failed || m_next_chunk >= m_chunk_count)
	{
		return false;
	}

	work.index = m_next_chunk++;
	work.body = m_body;
	work.context = m_context;
	compute_chunk_range(
		m_count,
		m_chunk_count,
		work.index,
		work.begin,
		work.end
	);
	++m_running;

	return true;
}

void thread_pool_implementation::finish_chunk(
	std::size_t chunk,
	std::exception_ptr exception
)
{
	bool finished;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (exception)
		{
			// The lowest numbered chunk wins rather than the first to arrive,
			// so that which error a caller is shown does not depend on which
			// core got there first.
			if (!m_exception || chunk < m_failed_chunk)
			{
				m_exception = std::move(exception);
				m_failed_chunk = chunk;
			}
			m_failed = true;
		}

		--m_running;
		finished = is_job_finished();
	}

	if (finished)
	{
		m_done.notify_one();
	}
}

bool thread_pool_implementation::is_job_finished() const noexcept
{
	return m_running == 0 && (m_failed || m_next_chunk >= m_chunk_count);
}

void thread_pool_implementation::stop_and_join() noexcept
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stop = true;
	}
	m_wake.notify_all();

	for (auto &worker : m_workers)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}
}

thread_pool::thread_pool(std::size_t worker_count)
	: m_implementation(
		std::make_unique<thread_pool_implementation>(worker_count)
	)
{
}

thread_pool::~thread_pool() = default;

std::size_t thread_pool::get_size() const noexcept
{
	return m_implementation->get_size();
}

void thread_pool::run(
	std::size_t count,
	std::size_t grain_size,
	body_function body,
	const void *context
)
{
	m_implementation->run(count, grain_size, body, context);
}

std::size_t thread_pool::get_default_worker_count()
{
	// Counting participants here rather than at the call site is what keeps
	// the subtraction next to the reason for it, and what keeps a count of
	// zero from wrapping into a request for every thread the machine can
	// address.
	std::size_t participants = std::thread::hardware_concurrency();
	if (participants == 0)
	{
		// hardware_concurrency() is allowed to give up, and there is nothing
		// better to assume than a single core when it does.
		participants = 1;
	}

	const char* environment_variable;
	if ((environment_variable = std::getenv(XMIPP4_NUM_THREADS_ENV_VARIABLE)))
	{
		char *end = nullptr;
		const auto parsed = std::strtoul(environment_variable, &end, 10);
		if (end != environment_variable && *end == '\0' && parsed > 0)
		{
			participants = static_cast<std::size_t>(parsed);
		}
		else
		{
			XMIPP4_LOG_WARN(
				"Ignoring {}=\"{}\": expected a positive integer.",
				XMIPP4_NUM_THREADS_ENV_VARIABLE,
				environment_variable
			);
		}
	}

	XMIPP4_ASSERT( participants > 0 );
	return participants - 1;
}

} // namespace cpu
} // namespace xmipp4
