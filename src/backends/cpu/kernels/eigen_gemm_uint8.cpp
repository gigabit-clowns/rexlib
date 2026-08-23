// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/kernels/eigen_gemm_impl.hpp>

#include <cstdint>

namespace rex
{
namespace cpu
{

REX_INSTANTIATE_EIGEN_GEMM(std::uint8_t);

} // namespace cpu
} // namespace rex
