// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_format_registry.hpp>

#include <em/image/core_image_format_registry.hpp>

/**
 * @brief Instantiate and auto-register an image read format.
 *
 * Write it once at the end of the format's translation unit. There is no
 * registrar list to add to, so a format is one new file and a format that
 * only reads never mentions the writing side.
 *
 * @param name Identifier of the registration object.
 * @param ... The format type. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REXLIB_REGISTER_IMAGE_READ_FORMAT(name, ...) \
	static const ::rexlib::em::image_format_registration< \
		__VA_ARGS__, \
		::rexlib::em::image_read_format_registry \
	> name##_image_read_format_registration( \
		::rexlib::em::get_core_image_read_format_registry() \
	)

/**
 * @brief Instantiate and auto-register an image write format.
 *
 * @param name Identifier of the registration object.
 * @param ... The format type.
 *
 * @see REXLIB_REGISTER_IMAGE_READ_FORMAT
 */
#define REXLIB_REGISTER_IMAGE_WRITE_FORMAT(name, ...) \
	static const ::rexlib::em::image_format_registration< \
		__VA_ARGS__, \
		::rexlib::em::image_write_format_registry \
	> name##_image_write_format_registration( \
		::rexlib::em::get_core_image_write_format_registry() \
	)
