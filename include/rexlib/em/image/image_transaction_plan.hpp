// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/index_table.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>
#include <rexlib/em/image/interned_path_list.hpp>

#include <cstddef>
#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief The regions to transfer between many files and one array.
 *
 * Every region pairs an offset into a file with an offset into the array,
 * names the file it belongs to, and shares one @ref image_transfer_shape with
 * every other region in the plan. The file rank of the shape is the rank of
 * every file.
 *
 * The shape is fixed when a plan is constructed; only the files and the
 * regions come and go.
 *
 * Regions are held in the order they were added, not grouped by file.
 *
 * @see image_transfer_plan
 */
class image_transaction_plan
{
public:
	/**
	 * @brief Construct an empty plan of a given shape.
	 *
	 * The shape is what every region of the plan shares and is fixed for
	 * the life of it; only the files and the regions are added and dropped.
	 *
	 * @param shape The shape of every region.
	 */
	REXLIB_API
	explicit image_transaction_plan(image_transfer_shape shape);

	REXLIB_API
	image_transaction_plan(const image_transaction_plan &other);
	REXLIB_API
	image_transaction_plan(image_transaction_plan &&other) noexcept;
	REXLIB_API
	~image_transaction_plan();

	REXLIB_API
	image_transaction_plan& operator=(const image_transaction_plan &other);
	REXLIB_API
	image_transaction_plan&
	operator=(image_transaction_plan &&other) noexcept;

	/**
	 * @brief Name a file the regions may address.
	 *
	 * A path equal to one already named yields the index it was given the
	 * first time, so the file of every region may be named without checking
	 * whether it has been named already.
	 *
	 * @param path Path to the file.
	 * @return std::size_t Index of the file, below @ref get_file_count.
	 */
	REXLIB_API
	std::size_t add_file(std::string path);

	/**
	 * @brief Append one region.
	 *
	 * @param file_index Index of the file it addresses, as @ref add_file
	 * returned it.
	 * @param file_offset Index of the first element of the region in the
	 * file. Its size must equal the file rank of the shape.
	 * @param array_offset Index of the first element of the region in the
	 * array. Its size must equal the array rank of the shape.
	 * @throws std::out_of_range If @p file_index names no file.
	 * @throws std::invalid_argument If either offset has the wrong rank.
	 */
	REXLIB_API
	void add(
		std::size_t file_index,
		span<const std::size_t> file_offset,
		span<const std::size_t> array_offset
	);

	/**
	 * @brief Drop the files and the regions, keeping the shape and the
	 * capacity.
	 */
	REXLIB_API
	void clear() noexcept;

	/**
	 * @brief Make room without allocating later.
	 *
	 * @param files Number of distinct files to make room for.
	 * @param regions Number of regions to make room for.
	 */
	REXLIB_API
	void reserve(std::size_t files, std::size_t regions);

	/**
	 * @brief Get how many regions are held.
	 *
	 * @return std::size_t The number of regions.
	 */
	REXLIB_API
	std::size_t get_region_count() const noexcept;

	/**
	 * @brief Get the shape every region shares.
	 *
	 * @return const image_transfer_shape& The shape. It refers to storage
	 * owned by this plan.
	 */
	REXLIB_API
	const image_transfer_shape& get_shape() const noexcept;

	/**
	 * @brief Get how many distinct files the regions may address.
	 *
	 * Counts every file named, including one no region addresses.
	 *
	 * @return std::size_t The number of files.
	 */
	REXLIB_API
	std::size_t get_file_count() const noexcept;

	/**
	 * @brief Get the path to one of the files.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return const std::string& The path. It refers to storage owned by
	 * this plan.
	 */
	REXLIB_API
	const std::string& get_file(std::size_t file_index) const noexcept;

	/**
	 * @brief Get which file one region addresses.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return std::size_t The file index, below @ref get_file_count.
	 */
	REXLIB_API
	std::size_t get_region_file(std::size_t region_index) const noexcept;

	/**
	 * @brief Get where a region starts in its file.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of the file rank of the
	 * shape. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_file_offset(std::size_t region_index) const noexcept;

	/**
	 * @brief Get where a region starts in the array.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of the array rank of the
	 * shape. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_array_offset(std::size_t region_index) const noexcept;

private:
	image_transfer_shape m_shape;
	interned_path_list m_files;
	index_table m_file_offsets;
	index_table m_array_offsets;
};

} // namespace em
} // namespace rexlib
