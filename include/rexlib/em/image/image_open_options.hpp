// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <ostream>

namespace rexlib
{
namespace em
{

/**
 * @brief How a dataset is going to be walked, stated when it is opened.
 *
 * The hint reaches the operating system as an access advice, which is set on
 * the open file rather than on each read. It is a hint in the strict sense:
 * every access remains legal whichever value is given.
 */
enum class image_access_hint
{
	/** @brief Nothing is known about the order of the accesses. */
	none,

	/** @brief Regions will be read roughly in increasing file order. */
	sequential,

	/** @brief Regions will be read in an unpredictable order. */
	random,
};

REXLIB_CONSTEXPR
const char* to_string(image_access_hint v) noexcept;

template<typename T>
std::basic_ostream<T>&
operator<<(std::basic_ostream<T>& os, image_access_hint v);

/**
 * @brief Everything that qualifies the opening of a dataset.
 *
 * A default constructed instance opens the first dataset of the file with no
 * hint, which is what a caller holding one image or one stack per file wants.
 */
class image_open_options
{
public:
	/**
	 * @brief Construct options selecting the first dataset with no hint.
	 */
	REXLIB_API
	image_open_options() noexcept;

	image_open_options(const image_open_options &other) noexcept = default;
	image_open_options(image_open_options &&other) noexcept = default;
	~image_open_options() = default;

	image_open_options&
	operator=(const image_open_options &other) noexcept = default;
	image_open_options&
	operator=(image_open_options &&other) noexcept = default;

	/**
	 * @brief Set which dataset of the file is opened.
	 *
	 * @param index Zero based index of the dataset.
	 */
	REXLIB_API
	void set_dataset_index(std::size_t index) noexcept;

	/**
	 * @brief Get which dataset of the file is opened.
	 *
	 * A reader exposes exactly one rectangular dataset, so a file holding
	 * several of them is opened once per dataset. A format holding only ever
	 * one rejects any index but zero.
	 *
	 * @return std::size_t The zero based index of the dataset.
	 */
	REXLIB_API
	std::size_t get_dataset_index() const noexcept;

	/**
	 * @brief Set how the dataset is going to be walked.
	 *
	 * @param hint The access hint.
	 */
	REXLIB_API
	void set_access_hint(image_access_hint hint) noexcept;

	/**
	 * @brief Get how the dataset is going to be walked.
	 *
	 * @return image_access_hint The access hint.
	 */
	REXLIB_API
	image_access_hint get_access_hint() const noexcept;

private:
	std::size_t m_dataset_index;
	image_access_hint m_access_hint;
};

} // namespace em
} // namespace rexlib

#include "image_open_options.inl"
