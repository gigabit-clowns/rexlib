// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/libraries/eigen/gemm/eigen_gemm_impl.hpp>

#include <complex>

namespace rexlib
{
namespace cpu
{

REXLIB_INSTANTIATE_EIGEN_GEMM(std::complex<float64_t>);

} // namespace cpu
} // namespace rexlib
