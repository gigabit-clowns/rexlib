// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_file_layout.hpp"
#include "image_file_mapping.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief Creates a file whose values are laid out as an
 * @ref image_file_layout states, and writes it through a mapping of it.
 *
 * The layout is complete when a writer is constructed, so the file is laid
 * out in full there and then: sized, and what precedes its values written.
 * A region can be written wherever it belongs and in any order afterwards,
 * and nothing about the file changes as it fills.
 *
 * A region that is never written keeps whatever the file was laid out with,
 * which is unspecified.
 *
 * A writer flushes itself when it is destroyed. A failure to do so then can
 * only be logged, so a writer whose failures matter is flushed beforehand.
 */
class mapped_image_writer final
	: public image_writer
{
public:
	/**
	 * @brief Create a file and map it for writing.
	 *
	 * Any file already at that path is replaced.
	 *
	 * @param path Path to the file to create.
	 * @param layout Where and how the file holds its values.
	 * @param preamble The bytes the file begins with, before its values.
	 * @throws std::invalid_argument If @p preamble reaches past where
	 * @p layout says the values begin.
	 * @throws image_file_error If the file could not be created, sized or
	 * mapped.
	 */
	mapped_image_writer(
		const std::string &path,
		image_file_layout layout,
		span<const byte> preamble
	);

	~mapped_image_writer() override;

	const image_descriptor& get_descriptor() const noexcept override;

	void write(
		const_array_ref source,
		const image_transfer_plan &regions
	) override;

	void flush() override;

private:
	image_file_layout m_layout;
	image_file_mapping m_mapping;
};

} // namespace em
} // namespace rexlib
