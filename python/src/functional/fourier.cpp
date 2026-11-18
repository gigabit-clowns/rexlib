// SPDX-License-Identifier: GPL-3.0-only

#include "fourier.hpp"

#include "../core/ndarray/array_ref_caster.hpp"

#include <rexlib/functional/fourier.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <nanobind/stl/vector.h>

#include <cstddef>
#include <vector>

namespace rexlib
{

namespace nb = nanobind;

namespace
{

using axis_list = std::vector<std::ptrdiff_t>;
using normalization = ops::fourier_normalization;

array py_fft(
	const_array_ref x,
	const axis_list &axes,
	normalization norm,
	const execution_context &context,
	array *out
)
{
	return fft(x, make_span(axes), norm, context, out);
}

array py_ifft(
	const_array_ref x,
	const axis_list &axes,
	normalization norm,
	const execution_context &context,
	array *out
)
{
	return ifft(x, make_span(axes), norm, context, out);
}

array py_rfft(
	const_array_ref x,
	const axis_list &axes,
	normalization norm,
	const execution_context &context,
	array *out
)
{
	return rfft(x, make_span(axes), norm, context, out);
}

array py_irfft(
	const_array_ref x,
	const axis_list &axes,
	std::size_t extent,
	normalization norm,
	const execution_context &context,
	array *out
)
{
	return irfft(x, make_span(axes), extent, norm, context, out);
}

array py_fftshift(
	const_array_ref x,
	const axis_list &axes,
	const execution_context &context,
	array *out
)
{
	return fftshift(x, make_span(axes), context, out);
}

array py_ifftshift(
	const_array_ref x,
	const axis_list &axes,
	const execution_context &context,
	array *out
)
{
	return ifftshift(x, make_span(axes), context, out);
}

} // anonymous namespace

void bind_fourier(nb::module_ &m)
{
	m.def(
		"fft", &py_fft,
		nb::arg("x"), nb::arg("axes"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"fft",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&fft),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ifft", &py_ifft,
		nb::arg("x"), nb::arg("axes"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ifft",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&ifft),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"rfft", &py_rfft,
		nb::arg("x"), nb::arg("axes"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"rfft",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&rfft),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"irfft", &py_irfft,
		nb::arg("x"), nb::arg("axes"), nb::arg("extent"),
		nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"irfft",
		nb::overload_cast<
			const_array_ref,
			std::size_t,
			normalization,
			const execution_context&,
			array*
		>(&irfft),
		nb::arg("x"), nb::arg("extent"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"fft2",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&fft2),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"fft3",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&fft3),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ifft2",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&ifft2),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ifft3",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&ifft3),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"rfft2",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&rfft2),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"rfft3",
		nb::overload_cast<
			const_array_ref,
			normalization,
			const execution_context&,
			array*
		>(&rfft3),
		nb::arg("x"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"irfft2",
		nb::overload_cast<
			const_array_ref,
			std::size_t,
			normalization,
			const execution_context&,
			array*
		>(&irfft2),
		nb::arg("x"), nb::arg("extent"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"irfft3",
		nb::overload_cast<
			const_array_ref,
			std::size_t,
			normalization,
			const execution_context&,
			array*
		>(&irfft3),
		nb::arg("x"), nb::arg("extent"), nb::arg("normalization"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"fftshift", &py_fftshift,
		nb::arg("x"), nb::arg("axes"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"fftshift",
		nb::overload_cast<
			const_array_ref,
			const execution_context&,
			array*
		>(&fftshift),
		nb::arg("x"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ifftshift", &py_ifftshift,
		nb::arg("x"), nb::arg("axes"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ifftshift",
		nb::overload_cast<
			const_array_ref,
			const execution_context&,
			array*
		>(&ifftshift),
		nb::arg("x"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
}

} // namespace rexlib
