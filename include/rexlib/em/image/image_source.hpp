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
 * Splits the plan by the file each region addresses and reads each
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
	 * A region transfers the intersection of what its file holds at its
	 * file offset with what @p destination holds at its array offset, up
	 * to the extents of the plan. The intersection begins where the region
	 * begins on either side, so a region reaching past either of them is
	 * shortened rather than refused, and one that reaches past both is
	 * shortened by whichever runs out first. A region reaching past a side
	 * along an axis the extents of the plan do not cover, which spans a
	 * single position, transfers nothing at all. The elements of
	 * @p destination no region reached are left as they were.
	 *
	 * A batch of patches is what this is for: a patch centred near a
	 * corner of an image begins outside it, which the array offset of its
	 * region carries, and only the part of it the image holds is read.
	 *
	 * The cost is that a @p destination too small for a region is
	 * shortened to fit instead of reported, since it is the same thing to
	 * this as a patch hanging over an edge.
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
	std::shared_ptr<image_reader_provider> m_readers;
	std::shared_ptr<rexlib::executor> m_executor;
};

} // namespace em
} // namespace rexlib
