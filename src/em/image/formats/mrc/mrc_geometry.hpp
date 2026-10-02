// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mrc_header.hpp"
#include "mrc_single_section.hpp"

#include <em/image/formats/memory_mapping/image_file_layout.hpp>

#include <rexlib/em/image/image_descriptor.hpp>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief Resolve what a header says about where and how a file holds its
 * values.
 *
 * An MRC file states its shape as three counts and a space group, and the
 * same three counts mean different things depending on that space group: a
 * stack of images and one volume of the same depth differ only in it, and a
 * stack of volumes divides the sections between its two leading axes, which
 * MRC2014 states by the space group alone, even for a stack of one volume or
 * of volumes one section deep. The one case the standard leaves open, a
 * single section in the image space group, is either one image or a stack of
 * one, and is settled by @p single_section.
 *
 * The axes of a stack come first, and the axes of one image or volume follow
 * in the order of the axes of space, the one along the first axis of space
 * last. The core rank is two for a file of images and three for one of
 * volumes.
 *
 * The values themselves are laid out with the columns changing fastest, and
 * the header names the axis of space the columns, the rows and the sections
 * each run along. A file that names them in another order is resolved with
 * the strides of its axes out of descending order rather than with its axes
 * transposed. One that names no axis at all is resolved as though it named
 * them in order.
 *
 * @param header The header of the file.
 * @param single_section What the file holds when the header states a single
 * section in the image space group.
 * @return image_file_layout The layout, in the byte order of the header.
 * @throws image_format_error If the axis correspondence of the header names
 * anything but the three axes of space, one each, without being unset, or if
 * the values of the file would not begin at an offset its elements can be
 * addressed at.
 */
image_file_layout derive_file_layout(
	const mrc_header &header,
	mrc_single_section single_section = mrc_single_section::image
);

/**
 * @brief Build the header of a file of a given shape.
 *
 * The inverse of @ref derive_file_layout: it decides the space group and the
 * sampling that state the difference the core rank of @p descriptor names,
 * since the extents alone do not say whether a file of @c (N,H,W) is a stack
 * of images or one volume. A stack of one image gets the header of a single
 * image, since MRC2014 has no other: which of the two the file holds is then
 * up to how it is read.
 *
 * Every field the format does not derive from the shape is left at what a
 * newly created file carries, save for a label naming the library that
 * built it.
 *
 * @param descriptor What the file holds.
 * @return mrc_header The header, in the byte order of the host.
 * @throws unsupported_operation_error If no MRC file has that shape, or if no
 * mode holds its data type.
 */
mrc_header make_header(const image_descriptor &descriptor);

} // namespace mrc
} // namespace em
} // namespace rexlib
