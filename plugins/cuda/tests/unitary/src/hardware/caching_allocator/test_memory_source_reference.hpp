// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/caching_allocator/memory_source.hpp>

namespace rexlib
{
namespace cuda
{

/**
 * @brief A @ref memory_source that forwards to one it does not own.
 *
 * An allocator takes ownership of its source, which a mock the test needs to
 * keep hold of cannot be given. This is what goes in instead.
 */
class memory_source_reference final
	: public memory_source
{
public:
	/**
	 * @brief Construct a reference to a source.
	 *
	 * @param target The source to forward to. Must outlive this.
	 */
	explicit memory_source_reference(memory_source &target) noexcept;
	~memory_source_reference() override;

	void* allocate(std::size_t size) override;
	void deallocate(void *data) noexcept override;
	bool is_host_accessible() const noexcept override;
	std::size_t get_base_alignment() const noexcept override;

private:
	memory_source *m_target;
};

} // namespace cuda
} // namespace rexlib
