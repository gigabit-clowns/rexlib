// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_sink.hpp>

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
 * Splits the plan by the file each region addresses and writes each
 * file's regions as one task, fanned out onto the executor this was
 * constructed with. Files are therefore written concurrently to
 * whatever degree the executor allows, not necessarily one after
 * another.
 *
 * Neither @ref completion::wait nor @ref completion::get of a completion
 * this sink returns may be called from within a task already running on
 * that executor.
 */
class REXLIB_API executor_image_sink final
	: public image_sink
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
	executor_image_sink(
		std::shared_ptr<image_writer_provider> writers,
		std::shared_ptr<rexlib::executor> executor
	);

	~executor_image_sink() override;

	std::shared_ptr<completion> write(
		const_array source,
		const image_transaction_plan &plan
	) const override;

	void flush() override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<image_writer_provider> m_writers;
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<rexlib::executor> m_executor;
};

} // namespace em
} // namespace rexlib
