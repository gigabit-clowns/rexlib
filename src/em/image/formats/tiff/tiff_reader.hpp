// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tiff_page_decoder.hpp"

#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <mutex>
#include <string>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief Reads a TIFF file whose pages are all alike.
 *
 * A file of one page is an image, and a file of several is a stack of them,
 * its first axis running along the pages.
 *
 * The samples of a file are compressed a block at a time, so a region is
 * not addressed where it lies: the blocks holding its rows are decoded and
 * the region is moved out of them. A read decodes a page at a time and only
 * the blocks its regions reach, and the page decoded last is kept, so
 * regions of one page read by successive calls decode each block once.
 *
 * Decoding goes through one handle on the file, so concurrent calls to
 * @ref read are serialised.
 */
class tiff_reader final
	: public image_reader
{
public:
	/**
	 * @brief Open a file for reading.
	 *
	 * @param path Path to the file.
	 * @throws image_file_error If the file can not be reached.
	 * @throws image_format_error If the file is not a TIFF file, if a page
	 * is one this format can not transfer, or if its pages differ in size
	 * or data type.
	 */
	explicit tiff_reader(const std::string &path);

	~tiff_reader() override = default;

	const image_descriptor& get_descriptor() const noexcept override;

	const image_metadata& get_metadata() const noexcept override;

	void read(
		array_ref destination,
		const image_transfer_plan &regions
	) const override;

private:
	mutable std::mutex m_mutex;
	mutable tiff_page_decoder m_decoder;
	image_descriptor m_descriptor;
	image_metadata m_metadata;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
