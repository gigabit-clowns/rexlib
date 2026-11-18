// SPDX-License-Identifier: GPL-3.0-only

#include "device_guard.hpp"

#include "error.hpp"

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

device_guard::device_guard(int ordinal)
	: m_previous(no_selection)
{
	int previous;
	REXLIB_CUDA_CHECK( cudaGetDevice(&previous) );

	if (previous != ordinal)
	{
		REXLIB_CUDA_CHECK( cudaSetDevice(ordinal) );
		m_previous = previous;
	}
}

device_guard::~device_guard()
{
	if (m_previous != no_selection)
	{
		REXLIB_CUDA_CHECK_NO_THROW( cudaSetDevice(m_previous) );
	}
}

} // namespace cuda
} // namespace rexlib
