// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/index_table.hpp>
#include <rexlib/em/image/interned_path_list.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief The regions transferred in one transaction.
 *
 * A transaction describes moving data between one array and one set of
 * files as a single whole.
 *
 * Every region pairs an ND offset into a file with an ND offset into the
 * array, names the file it belongs to, and shares one set of extents with
 * every other region in the plan.
 *
 * The extents are the shape of one region, so their rank is the rank of the
 * region rather than of either side, exactly as in @ref image_transfer_plan.
 * A side of higher rank spans a single position along the axes the extents
 * don't reach; those axes are implicitly padded with leading ones.
 *
 * The extents and each side's ranks is fixed when a plan is constructed and
 * never changes afterward, so a plan is always a complete, ready-to-use object
 * rather than something configured before use. Only its files and regions come
 * and go, and every file must share the same rank.
 *
 * Regions are held in the order they were added. A consumer that wants to walk
 * them one file at a time builds that ordering alongside the plan rather than
 * relying on the plan for it, keeping the plan itself limited to what is
 * transferred and nothing else.
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
	 * @param extents Extents of one region. Their rank is the rank of the
	 * region, which may be lower than that of either side.
	 * @param file_rank Rank of every file the regions address.
	 * @param array_rank Rank of the array the regions address.
	 * @throws std::invalid_argument If the rank of @p extents exceeds
	 * @p file_rank or @p array_rank.
	 */
	REXLIB_API
	image_transaction_plan(
		span<const std::size_t> extents,
		std::size_t file_rank,
		std::size_t array_rank
	);

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
	 * first time, so a caller may name the file of every region without
	 * checking whether it has been named already.
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
	 * file. Its size must equal @ref get_file_rank.
	 * @param array_offset Index of the first element of the region in the
	 * array. Its size must equal @ref get_array_rank.
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
	 * @brief Get the rank of one region.
	 *
	 * The rank of the extents. Each side spans a single position along the
	 * axes beyond it.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_rank() const noexcept;

	/**
	 * @brief Get the rank of every file the regions address.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_file_rank() const noexcept;

	/**
	 * @brief Get the rank of the array the regions address.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_array_rank() const noexcept;

	/**
	 * @brief Get the extents shared by every region.
	 *
	 * @return span<const std::size_t> The extents, of rank @ref get_rank.
	 */
	REXLIB_API
	span<const std::size_t> get_extents() const noexcept;

	/**
	 * @brief Get how many distinct files the regions may address.
	 *
	 * Counts the files that were named, which a file no region ended up
	 * addressing is still one of.
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
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_file_rank. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_file_offset(std::size_t region_index) const noexcept;

	/**
	 * @brief Get where a region starts in the array.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_array_rank. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_array_offset(std::size_t region_index) const noexcept;

private:
	std::vector<std::size_t> m_extents;
	interned_path_list m_files;
	index_table m_file_offsets;
	index_table m_array_offsets;
};

} // namespace em
} // namespace rexlib
