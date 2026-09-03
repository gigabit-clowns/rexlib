// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_writer_provider.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_metadata;
class image_write_format_manager;

/**
 * @brief A provider whose files are declared and closed by their owner.
 *
 * The write side can not do what @ref caching_image_reader_provider does.
 * Creating a file replaces whatever was there, so a writer dropped and
 * reopened would truncate everything already written to it; an eviction
 * policy is therefore not a policy but a bug. What files exist and when each
 * is finished is instead said outright, through @ref declare and
 * @ref close.
 *
 * The declared files are content rather than shape — they come and go over
 * the life of a provider — so only the format manager is a constructor
 * parameter.
 *
 * Each file therefore has one lifecycle, **declare, acquire as often as
 * needed, close**. A file is opened when it is first acquired, so declaring
 * a thousand output stacks costs no file descriptors until each is reached,
 * and @ref close is what gives them back.
 *
 * @par Thread safety
 * Every method may be called concurrently. Unlike the reading side, the lock
 * *is* held while a file is created, since two callers racing to open one
 * file would otherwise each replace it. Acquiring an already open writer
 * takes the lock only long enough to look it up.
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
	 * Records what the file will be, stated exactly as
	 * @ref image_write_format::open states it, since that is what this will
	 * eventually be handed. Nothing is created until the file is first
	 * acquired, so this costs no file descriptor and can not fail on
	 * anything the file system has to say.
	 *
	 * A path that is still declared is refused rather than replaced,
	 * because replacing it would silently strand whatever had been written
	 * to the file it names. @ref close it first, which says so.
	 *
	 * @param path Path to the file to create.
	 * @param extents Extents of the file, slowest axis first.
	 * @param core_rank How many trailing extents are one image or volume,
	 * the leading ones being the axes the file stacks along. Two for a stack
	 * of images, three for one volume.
	 * @param data_type Data type of its elements.
	 * @param metadata How its samples map onto physical space.
	 * @throws std::invalid_argument If @p core_rank is zero or exceeds the
	 * rank of @p extents.
	 * @throws invalid_operation_error If that path is already declared.
	 */
	REXLIB_API
	void declare(
		std::string path,
		span<const std::size_t> extents,
		std::size_t core_rank,
		numerical_type data_type,
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
	 * @throws image_format_error If the pending writes could not be
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
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get a writer over one declared file, creating it on the first
	 * call.
	 *
	 * @param path Path to the file to write.
	 * @return std::shared_ptr<image_writer> The writer, never null, and the
	 * same one every time until the file is closed.
	 * @throws std::out_of_range If that path is not declared.
	 * @throws invalid_operation_error If no format can create the file.
	 * @throws image_format_error If the file could not be created.
	 */
	REXLIB_API
	std::shared_ptr<image_writer> acquire(const std::string &path) override;

	/**
	 * @brief Flush every writer this has created.
	 *
	 * Declared files that were never acquired are not created by this.
	 *
	 * @throws image_format_error If the pending writes could not be
	 * completed.
	 */
	REXLIB_API
	void flush() override;

private:
	class implementation;
	REXLIB_STD_MEMBER_INTERFACE
	std::unique_ptr<implementation> m_implementation;
};

} // namespace em
} // namespace rexlib
