// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/kernels/eigen_gemm_impl.hpp>

#include <complex>

namespace xmipp4
{
namespace cpu
{

REX_INSTANTIATE_EIGEN_GEMM(std::complex<float64_t>);

} // namespace cpu
} // namespace xmipp4
