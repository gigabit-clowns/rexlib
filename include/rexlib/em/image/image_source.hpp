// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

class array;
class completion;
class executor;

namespace em
{

class image_reader_provider;
class image_transaction_plan;

/**
 * @brief Executes a transaction plan by reading every file it names.
 *
 * Splits the plan by file with @ref region_grouping and reads each
 * file's regions as one task, fanned out onto the executor this was
 * constructed with. Files are therefore read concurrently to whatever
 * degree the executor allows, not necessarily one after another.
 *
 * @par Thread safety
 * read may be called concurrently.
 */
class image_source
{
public:
	/**
	 * @brief Construct a source over a provider and an executor.
	 *
	 * @param readers Where a path becomes an open reader.
	 * @param executor Where a file's read is run.
	 * @throws std::invalid_argument If @p readers or @p executor is
	 * null.
	 */
	REXLIB_API
	image_source(
		std::shared_ptr<image_reader_provider> readers,
		std::shared_ptr<rexlib::executor> executor
	);

	image_source(const image_source &other) = delete;
	image_source(image_source &&other) = delete;

	REXLIB_API
	~image_source();

	image_source& operator=(const image_source &other) = delete;
	image_source& operator=(image_source &&other) = delete;

	/**
	 * @brief Read every region a transaction plan names.
	 *
	 * Returns before the reads are done. Neither @ref completion::wait
	 * nor @ref completion::get of the completion returned may be called
	 * from within a task already running on the executor this source
	 * was constructed with.
	 *
	 * @param destination Where the regions land.
	 * @param plan The transaction to read.
	 * @return std::shared_ptr<completion> The completion, never null.
	 */
	REXLIB_API
	std::shared_ptr<completion> read(
		array destination,
		const image_transaction_plan &plan
	) const;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<image_reader_provider> m_readers;
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<rexlib::executor> m_executor;
};

} // namespace em
} // namespace rexlib
