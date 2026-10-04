// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tiff_file.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief Creates a TIFF file and writes it a page at a time.
 *
 * A file of rank two is one page, and a file of rank three is a stack of
 * them, its first axis running along the pages. Pages of integer samples
 * are compressed with LZW and pages of floating point ones are left as they
 * are. The file is created as BigTIFF when its samples would not be sure to
 * fit in a classic one.
 *
 * A page is encoded as a whole and appended to the file, which is what a
 * compressed file allows and no more. A write is therefore narrower than
 * what @ref image_writer states:
 *
 * - Every region covers whole pages: one page, or a run of consecutive
 *   ones. A region that covers part of a page is refused.
 * - Pages are written in order and once. The regions of one call may be
 *   stated in any order, but together they cover a run of consecutive pages
 *   that begins at the first page not yet written. The file may be written
 *   across any number of calls that follow one another this way.
 * - Concurrent calls are serialised, and one that arrives out of turn is
 *   refused like any other that does not begin at the next page.
 *
 * A call that is refused writes nothing.
 *
 * A page is handed to the operating system by the call that writes it, so
 * nothing is left pending for @ref flush. Pages that are never written are
 * absent from the file, which then holds fewer pages than it was created
 * for; a writer destroyed in that state logs it.
 */
class tiff_writer final
	: public image_writer
{
public:
	/**
	 * @brief Create a file and open it for writing.
	 *
	 * Any file already at that path is replaced.
	 *
	 * @param path Path to the file to create.
	 * @param descriptor What the file holds.
	 * @throws std::invalid_argument If @p descriptor is not of rank two or
	 * three, if it is not of core rank two, or if it has an extent of zero.
	 * @throws unsupported_operation_error If no sample holds the data type
	 * of @p descriptor.
	 * @throws image_file_error If the file could not be created.
	 */
	tiff_writer(const std::string &path, image_descriptor descriptor);

	~tiff_writer() override;

	const image_descriptor& get_descriptor() const noexcept override;

	void write(
		const_array_ref source,
		const image_transfer_plan &regions
	) override;

	void flush() override;

private:
	image_descriptor m_descriptor;
	std::mutex m_mutex;
	tiff_file m_file;
	std::size_t m_page_count;
	std::size_t m_next_page;
	std::vector<byte> m_page;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
