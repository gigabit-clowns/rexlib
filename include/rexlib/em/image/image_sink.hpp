// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

class completion;
class const_array;
class executor;

namespace em
{

class image_transaction_plan;
class image_writer_provider;

/**
 * @brief Executes a transaction plan by writing every file it names.
 *
 * Splits the plan by file with @ref region_grouping and writes each
 * file's regions as one task, fanned out onto the executor this was
 * constructed with. Files are therefore written concurrently to
 * whatever degree the executor allows, not necessarily one after
 * another.
 *
 * @par Thread safety
 * write may be called concurrently.
 */
class image_sink
{
public:
	/**
	 * @brief Construct a sink over a provider and an executor.
	 *
	 * @param writers Where a path becomes an open writer.
	 * @param executor Where a file's write is run.
	 * @throws std::invalid_argument If @p writers or @p executor is
	 * null.
	 */
	REXLIB_API
	image_sink(
		std::shared_ptr<image_writer_provider> writers,
		std::shared_ptr<rexlib::executor> executor
	);

	image_sink(const image_sink &other) = delete;
	image_sink(image_sink &&other) = delete;

	REXLIB_API
	~image_sink();

	image_sink& operator=(const image_sink &other) = delete;
	image_sink& operator=(image_sink &&other) = delete;

	/**
	 * @brief Write every region a transaction plan names.
	 *
	 * Returns before the writes are done. Neither @ref completion::wait
	 * nor @ref completion::get of the completion returned may be called
	 * from within a task already running on the executor this sink was
	 * constructed with.
	 *
	 * @param source The values to write.
	 * @param plan The transaction to write.
	 * @return std::shared_ptr<completion> The completion, never null.
	 */
	REXLIB_API
	std::shared_ptr<completion> write(
		const_array source,
		const image_transaction_plan &plan
	);

	/**
	 * @brief Make everything written through this sink reach storage.
	 *
	 * Call only after waiting on or getting any @ref write completions
	 * you care about, matching @ref image_writer_provider::flush's own
	 * contract.
	 *
	 * @throws image_format_error If the pending writes could not be
	 * completed.
	 */
	REXLIB_API
	void flush();

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<image_writer_provider> m_writers;
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<rexlib::executor> m_executor;
};

} // namespace em
} // namespace rexlib
