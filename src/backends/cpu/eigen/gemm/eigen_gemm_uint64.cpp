// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/eigen/gemm/eigen_gemm_impl.hpp>

#include <cstdint>

namespace rexlib
{
namespace cpu
{

REXLIB_INSTANTIATE_EIGEN_GEMM(std::uint64_t);

} // namespace cpu
} // namespace rexlib
