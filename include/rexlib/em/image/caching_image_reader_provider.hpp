// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief A provider that keeps the readers it was last asked for.
 *
 * Opening a file costs a system call and a header read, which asking for
 * the same few files over and over repeats for nothing. This keeps a bounded
 * number of readers open, keyed by path, and evicts the least recently asked
 * for when it is full. A path it holds is served without opening anything,
 * and becomes the last to be evicted.
 *
 * It is a **decorator over another provider**, not a second opener: on a
 * miss it asks its backing provider and keeps what it gets, so it caches the
 * readers of any provider alike.
 *
 * The capacity is a file descriptor budget, which is why it is stated in
 * entries rather than in bytes.
 *
 * @par Thread safety
 * @ref acquire may be called concurrently. The lock is not held while the
 * backing provider opens a file, so two concurrent calls missing on one
 * path may both open it and one of the two readers is kept; that wastes an
 * open and never costs correctness, since either reader serves equally.
 * Handing out @c shared_ptr means the one not kept stays alive as long as it
 * is read through, and so does an evicted one.
 */
class REXLIB_API caching_image_reader_provider final
	: public image_reader_provider
{
public:
	/**
	 * @brief Construct a cache in front of another provider.
	 *
	 * @param backing The provider asked on a miss.
	 * @param capacity Most readers to keep open at once. Must be greater
	 * than zero.
	 * @throws std::invalid_argument If @p backing is null or @p capacity is
	 * zero.
	 */
	caching_image_reader_provider(
		std::shared_ptr<image_reader_provider> backing,
		std::size_t capacity
	);

	~caching_image_reader_provider() override;

	/**
	 * @brief Get the number of readers this may keep open at once.
	 *
	 * @return std::size_t The capacity, in entries.
	 */
	std::size_t get_capacity() const noexcept;

	/**
	 * @brief Get how many readers are kept open.
	 *
	 * @return std::size_t The number of entries, never above
	 * @ref get_capacity.
	 */
	std::size_t get_reader_count() const noexcept;

	std::shared_ptr<const image_reader>
	acquire(const std::string &path) override;

private:
	class implementation;
	REXLIB_STD_MEMBER_INTERFACE
	std::unique_ptr<implementation> m_implementation;
};

} // namespace em
} // namespace rexlib
