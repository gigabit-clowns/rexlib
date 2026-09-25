// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_writer_provider.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_descriptor;
class image_metadata;
class image_write_format_manager;

/**
 * @brief A provider whose files are declared and closed by their owner.
 *
 * Creating a file replaces whatever was there, so a writer dropped and
 * reopened would truncate everything already written to it, and no eviction
 * policy can decide when that is safe. Which files exist and when each is
 * finished is instead said outright, through @ref declare and @ref close.
 *
 * Each file therefore has one lifecycle, **declare, acquire as often as
 * needed, close**:
 * - A declared file is created, through the format manager, the first time
 *   it is acquired, so declaring many files costs no file descriptor until
 *   each is reached. Every later acquire returns that same writer until the
 *   file is closed, and a path never declared is refused with
 *   @c std::out_of_range.
 * - @ref close flushes and drops the writer, which is what gives its file
 *   descriptor back.
 * - Flushing the provider flushes every writer it has created, and creates
 *   none: a declared file never acquired stays uncreated.
 *
 * @par Thread safety
 * Every method may be called concurrently. The lock is held while a file is
 * created, since two concurrent calls creating one file would otherwise each
 * replace it, and only long enough to look it up when it is already open.
 */
class managed_image_writer_provider final
	: public image_writer_provider
{
public:
	/**
	 * @brief Construct a provider creating files through a format manager.
	 *
	 * @param formats The formats a file may be created with.
	 * @throws std::invalid_argument If @p formats is null.
	 */
	REXLIB_API
	explicit managed_image_writer_provider(
		std::shared_ptr<const image_write_format_manager> formats
	);

	REXLIB_API
	~managed_image_writer_provider() override;

	/**
	 * @brief Make a file writable.
	 *
	 * Records the descriptor and metadata the file will be created with.
	 * Nothing is created until the file is first acquired, so this costs no
	 * file descriptor and can not fail on anything the file system has to
	 * say.
	 *
	 * A path that is still declared is refused rather than replaced,
	 * because replacing it would silently strand whatever had been written
	 * to the file it names. @ref close it first, which says so.
	 *
	 * @param path Path to the file to create.
	 * @param descriptor What the file holds.
	 * @param metadata How its samples map onto physical space.
	 * @throws std::logic_error If that path is already declared.
	 */
	REXLIB_API
	void declare(
		std::string path,
		image_descriptor descriptor,
		const image_metadata &metadata
	);

	/**
	 * @brief Finish a file.
	 *
	 * Flushes the writer if the file was ever acquired, drops it, and
	 * forgets the declaration. Acquiring that path afterwards throws as an
	 * undeclared one does, and declaring it again creates the file afresh.
	 *
	 * Closing a file that was declared but never acquired creates nothing
	 * and simply forgets it.
	 *
	 * @param path Path to the file to finish.
	 * @throws std::out_of_range If that path is not declared.
	 * @throws image_file_error If the pending writes could not be
	 * completed.
	 */
	REXLIB_API
	void close(const std::string &path);

	/**
	 * @brief Get how many files are declared.
	 *
	 * Counts what has been declared and not yet closed, whether or not it
	 * has been acquired.
	 *
	 * @return std::size_t The number of files.
	 */
	REXLIB_API
	std::size_t get_file_count() const noexcept;

	REXLIB_API
	std::shared_ptr<image_writer> acquire(const std::string &path) override;

	REXLIB_API
	void flush() override;

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;
};

} // namespace em
} // namespace rexlib
