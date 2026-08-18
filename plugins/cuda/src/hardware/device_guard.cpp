// SPDX-License-Identifier: GPL-3.0-only

#include "device_guard.hpp"

#include "error.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace cuda
{

device_guard::device_guard(int ordinal)
	: m_previous(no_selection)
{
	int previous;
	XMIPP4_CUDA_CHECK( cudaGetDevice(&previous) );

	if (previous != ordinal)
	{
		XMIPP4_CUDA_CHECK( cudaSetDevice(ordinal) );
		m_previous = previous;
	}
}

device_guard::~device_guard()
{
	if (m_previous != no_selection)
	{
		XMIPP4_CUDA_CHECK_NO_THROW( cudaSetDevice(m_previous) );
	}
}

} // namespace cuda
} // namespace xmipp4
