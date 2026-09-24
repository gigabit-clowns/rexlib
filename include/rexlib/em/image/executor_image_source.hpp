// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_source.hpp>

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
 * Neither @ref completion::wait nor @ref completion::get of a completion
 * this source returns may be called from within a task already running on
 * that executor.
 */
class REXLIB_API executor_image_source final
	: public image_source
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
	executor_image_source(
		std::shared_ptr<image_reader_provider> readers,
		std::shared_ptr<rexlib::executor> executor
	);

	~executor_image_source() override;

	std::shared_ptr<completion> read(
		array destination,
		const image_transaction_plan &plan
	) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<image_reader_provider> m_readers;
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<rexlib::executor> m_executor;
};

} // namespace em
} // namespace rexlib
