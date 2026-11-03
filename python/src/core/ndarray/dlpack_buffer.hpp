// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/buffer.hpp>

#include <dlpack/dlpack.h>

#include <cstddef>

namespace rexlib
{

/**
 * Presents the memory of a tensor received through DLPack as a buffer. The
 * buffer is host memory. It owns the tensor, and gives it back to whoever
 * produced it when it dies.
 */
class dlpack_buffer final
	: public buffer
{
public:
	/**
	 * Takes a tensor over. `data` is the first byte of the memory the tensor
	 * describes and `size` how many bytes of it there are.
	 */
	dlpack_buffer(
		DLManagedTensor *tensor,
		void *data,
		std::size_t size
	) noexcept;
	dlpack_buffer(
		DLManagedTensorVersioned *tensor,
		void *data,
		std::size_t size
	) noexcept;
	~dlpack_buffer() override;

	void* get_host_ptr() noexcept override;
	const void* get_host_ptr() const noexcept override;
	std::size_t get_size() const noexcept override;
	const memory_resource& get_memory_resource() const noexcept override;

private:
	using release_function = void (*)(void *tensor);

	void *m_tensor;
	release_function m_release;
	void *m_data;
	std::size_t m_size;
};

} // namespace rexlib
