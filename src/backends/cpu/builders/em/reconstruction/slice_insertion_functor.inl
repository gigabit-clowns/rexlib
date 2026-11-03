// SPDX-License-Identifier: GPL-3.0-only

#include "slice_insertion_functor.hpp"

#include "alternating_sign.hpp"
#include "row_insertion_kernel.hpp"
#include "tabulated_phase.hpp"
#include "virtual_row.hpp"

#include <backends/cpu/kernels/axis_phase.hpp>
#include <backends/cpu/kernels/box_window.hpp>
#include <backends/cpu/kernels/kaiser_bessel_window.hpp>
#include <backends/cpu/kernels/row_interval.hpp>
#include <backends/cpu/kernels/triangle_window.hpp>
#include <backends/cpu/loops/claimed_loop.hpp>
#include <backends/cpu/plans/volume_tiling.hpp>

#include <rexlib/backends/cpu/parallel_for.hpp>
#include <rexlib/core/layout/joint_cursor.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace rexlib
{
namespace cpu
{

template <typename T, typename SampleWeight>
inline
slice_insertion_functor<T, SampleWeight>::slice_insertion_functor(
	slice_insertion_layout_plan plan,
	const em::gridding_window &window,
	const parameters_type &parameters,
	const slice_insertion_tuning &tuning
)
	: m_plan(std::move(plan))
	, m_window(window)
	, m_parameters(parameters)
	, m_tuning(tuning)
{
}

template <typename T, typename SampleWeight>
template <typename... Outs, typename... Ins>
inline
void slice_insertion_functor<T, SampleWeight>::operator()(
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs,
	std::tuple<>,
	thread_pool &pool
) const
{
	switch (m_window.get_profile())
	{
	case em::gridding_profile::box:
		run_with_window(box_window(), outputs, inputs, pool);
		break;
	case em::gridding_profile::triangle:
		run_with_window(triangle_window(), outputs, inputs, pool);
		break;
	case em::gridding_profile::kaiser_bessel:
		run_with_window(
			kaiser_bessel_window<T>(m_window),
			outputs,
			inputs,
			pool
		);
		break;
	}
}

template <typename T, typename SampleWeight>
template <typename Window, typename... Outs, typename... Ins>
inline
void slice_insertion_functor<T, SampleWeight>::run_with_window(
	const Window &window,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs,
	thread_pool &pool
) const
{
	auto *numerator = std::get<0>(outputs);
	auto *denominator = std::get<1>(outputs);
	const auto extent = m_plan.get_volume_extent();
	if (extent % 2 == 0)
	{
		run(window, alternating_sign(), numerator, denominator, inputs, pool);
	}
	else
	{
		run(
			window,
			tabulated_phase<T>(extent),
			numerator,
			denominator,
			inputs,
			pool
		);
	}
}

template <typename T, typename SampleWeight>
template <typename Window, typename VolumePhase, typename... Ins>
inline
void slice_insertion_functor<T, SampleWeight>::run(
	const Window &window,
	const VolumePhase &phase,
	complex_type *numerator,
	T *denominator,
	const std::tuple<const Ins*...> &inputs,
	thread_pool &pool
) const
{
	std::vector<slice_entry> entries;
	std::vector<slice_state_type> states;
	std::vector<complex_type> phases;
	settle_slices(inputs, entries, states, phases, pool);

	const auto extent = m_plan.get_volume_extent();
	const auto half_extent = extent / 2 + 1;
	const auto row_bytes = half_extent * (sizeof(complex_type) + sizeof(T));
	const volume_tiling tiling(
		extent,
		choose_tile_extent(
			extent,
			row_bytes,
			m_tuning.get_tile_byte_budget(),
			m_tuning.get_tiles_per_thread() * pool.get_size(),
			m_window.get_half_width()
		)
	);

	const auto tile_count = tiling.get_tile_count();
	run_claimed_loop(
		pool,
		m_plan.get_kept_count() * tile_count,
		[&](std::size_t task)
		{
			insert_tile(
				window,
				phase,
				numerator,
				denominator,
				inputs,
				entries,
				states,
				phases,
				task / tile_count,
				tiling.get_tile(task % tile_count)
			);
		}
	);
}

template <typename T, typename SampleWeight>
template <typename... Ins>
inline
void slice_insertion_functor<T, SampleWeight>::settle_slices(
	const std::tuple<const Ins*...> &inputs,
	std::vector<slice_entry> &entries,
	std::vector<slice_state_type> &states,
	std::vector<complex_type> &phases,
	thread_pool &pool
) const
{
	const auto n = m_plan.get_slice_extent();
	const auto h = n / 2 + 1;
	const auto table_size = h + n;
	const auto reduced_count = m_plan.get_reduced_count();
	const auto count = m_plan.get_kept_count() * reduced_count;
	const auto scale = static_cast<double>(m_plan.get_volume_extent()) /
		static_cast<double>(n);
	const auto centre = static_cast<double>(n / 2);

	entries.resize(count);
	states.resize(count);
	phases.resize(count * table_size);

	const auto &kept_layout = m_plan.get_kept_layout();
	const auto &reduced_layout = m_plan.get_reduced_layout();
	const auto &transform_core =
		m_plan.get_input_core(SampleWeight::transform_index);
	const auto &radial_core = m_plan.get_input_core(SampleWeight::radial_index);
	const auto &extra_core = m_plan.get_input_core(SampleWeight::extra_index);
	const auto *transforms = std::get<SampleWeight::transform_index>(inputs);
	const auto *radials = std::get<SampleWeight::radial_index>(inputs);
	const auto *extras = std::get<SampleWeight::extra_index>(inputs);

	parallel_for(
		pool,
		count,
		16,
		[&](std::size_t begin, std::size_t end)
		{
			joint_cursor kept_cursor;
			joint_cursor reduced_cursor;
			for (auto index = begin; index < end; ++index)
			{
				kept_layout.seek(kept_cursor, index / reduced_count);
				reduced_layout.seek(reduced_cursor, index % reduced_count);
				const auto kept_offsets = kept_cursor.get_offsets();
				const auto reduced_offsets = reduced_cursor.get_offsets();
				const auto offset_of = [&](std::size_t input)
				{
					return kept_offsets[input] + reduced_offsets[input];
				};

				const auto *transform =
					transforms + offset_of(SampleWeight::transform_index);
				const auto row_stride = transform_core.get_stride(0);
				const auto column_stride = transform_core.get_stride(1);
				const auto at = [&](std::size_t row, std::size_t column)
				{
					return static_cast<double>(transform[
						static_cast<std::ptrdiff_t>(row) * row_stride +
						static_cast<std::ptrdiff_t>(column) * column_stride
					]);
				};
				const slice_frame frame(
					{{ at(0, 0), at(0, 1), at(0, 2) }},
					{{ at(1, 0), at(1, 1), at(1, 2) }},
					scale
				);

				const auto radial_offset =
					offset_of(SampleWeight::radial_index);
				const auto *radial = radials + radial_offset;
				std::ptrdiff_t last_shell = -1;
				for (std::size_t shell = 0; shell < h; ++shell)
				{
					const auto weight = radial[
						static_cast<std::ptrdiff_t>(shell) *
						radial_core.get_stride(0)
					];
					if (weight != T(0))
					{
						last_shell = static_cast<std::ptrdiff_t>(shell);
					}
				}

				const auto phase_offset = index * table_size;
				fill_axis_phase(
					&phases[phase_offset], h, n, centre + at(0, 3)
				);
				fill_axis_phase(
					&phases[phase_offset + h], n, n, centre + at(1, 3)
				);

				entries[index] = slice_entry(
					frame,
					last_shell,
					offset_of(2),
					radial_offset,
					phase_offset
				);
				states[index] = slice_state_type(
					m_parameters,
					n,
					extras + offset_of(SampleWeight::extra_index),
					extra_core
				);
			}
		}
	);
}

template <typename T, typename SampleWeight>
template <typename Window, typename VolumePhase, typename... Ins>
inline
void slice_insertion_functor<T, SampleWeight>::insert_tile(
	const Window &window,
	const VolumePhase &phase,
	complex_type *numerator,
	T *denominator,
	const std::tuple<const Ins*...> &inputs,
	const std::vector<slice_entry> &entries,
	const std::vector<slice_state_type> &states,
	const std::vector<complex_type> &phases,
	std::size_t kept,
	const volume_tile &tile
) const
{
	const auto input_count = sizeof...(Ins);
	const auto &kept_layout = m_plan.get_kept_layout();
	joint_cursor cursor;
	kept_layout.seek(cursor, kept);
	const auto offsets = cursor.get_offsets();

	auto *numerator_tile = numerator + offsets[input_count];
	auto *denominator_tile = denominator + offsets[input_count + 1];
	const auto *numerator_accumulator = std::get<0>(inputs) + offsets[0];
	const auto *denominator_accumulator = std::get<1>(inputs) + offsets[1];
	if (
		!m_plan.output_matches_accumulator(0) ||
		numerator_tile != numerator_accumulator
	)
	{
		copy_tile(
			numerator_tile,
			m_plan.get_output_core(0),
			numerator_accumulator,
			m_plan.get_input_core(0),
			tile
		);
	}
	if (
		!m_plan.output_matches_accumulator(1) ||
		denominator_tile != denominator_accumulator
	)
	{
		copy_tile(
			denominator_tile,
			m_plan.get_output_core(1),
			denominator_accumulator,
			m_plan.get_input_core(1),
			tile
		);
	}

	row_insertion_kernel<T, Window, VolumePhase, SampleWeight> kernel(
		window,
		phase,
		m_plan.get_volume_extent(),
		m_plan.get_output_core(0),
		m_plan.get_output_core(1)
	);
	kernel.bind(numerator_tile, denominator_tile);

	volume_tile::interval_list_type z_intervals;
	volume_tile::interval_list_type y_intervals;
	const auto z_count = tile.list_z_frequencies(z_intervals);
	const auto y_count = tile.list_y_frequencies(y_intervals);

	const auto n = m_plan.get_slice_extent();
	const auto h = n / 2 + 1;
	const auto signed_n = static_cast<std::ptrdiff_t>(n);
	const auto last_column =
		static_cast<std::ptrdiff_t>(h) - (n % 2 == 0 ? 2 : 1);
	const auto &slice_core = m_plan.get_input_core(2);
	const auto &radial_core = m_plan.get_input_core(SampleWeight::radial_index);
	const auto *slices = std::get<2>(inputs);
	const auto *radials = std::get<SampleWeight::radial_index>(inputs);
	const auto reduced_count = m_plan.get_reduced_count();

	for (std::size_t reduced = 0; reduced < reduced_count; ++reduced)
	{
		const auto index = kept * reduced_count + reduced;
		const auto &entry = entries[index];
		const auto last_shell = entry.get_last_shell();
		if (last_shell < 0)
		{
			continue;
		}

		const auto &frame = entry.get_frame();
		const auto *slice = slices + entry.get_slice_offset();
		const auto *radial = radials + entry.get_radial_offset();
		const auto *x_phases = &phases[entry.get_phase_offset()];
		const auto *y_phases = x_phases + h;
		const auto radius = static_cast<double>(last_shell) + 0.5;
		const auto widest =
			std::min(last_column, static_cast<std::ptrdiff_t>(radius));

		// The rows of an even extent stop short of its Nyquist frequency.
		const auto highest_row = n % 2 == 0 ? signed_n / 2 - 1 : signed_n / 2;
		const auto first_row = std::max(-highest_row, -widest);
		const auto last_row = std::min(highest_row, widest);

		for (int sign = 1; sign >= -1; sign -= 2)
		{
			const bool negated = sign < 0;
			const std::ptrdiff_t first = negated ? 1 : 0;
			const auto signed_scale = static_cast<double>(sign);
			const auto &x_frame = frame.get_x_step();
			const auto &y_frame = frame.get_y_step();
			const volume_frequency step = {{
				signed_scale * x_frame[0],
				signed_scale * x_frame[1],
				signed_scale * x_frame[2]
			}};
			const volume_frequency y_step = {{
				signed_scale * y_frame[0],
				signed_scale * y_frame[1],
				signed_scale * y_frame[2]
			}};

			for (std::size_t zi = 0; zi < z_count; ++zi)
			{
				for (std::size_t yi = 0; yi < y_count; ++yi)
				{
					kernel.set_box(z_intervals[zi], y_intervals[yi]);
					const auto rows = bound_rows(
						step,
						y_step,
						kernel.get_reach(),
						first,
						widest,
						first_row,
						last_row
					);

					for (auto k_y = rows.get_begin();
						k_y < rows.get_end();
						++k_y)
					{
						const auto k_y_value = static_cast<double>(k_y);
						const auto reach =
							radius * radius - k_y_value * k_y_value;
						if (reach < 0.0)
						{
							continue;
						}
						const auto last = std::min(
							widest,
							static_cast<std::ptrdiff_t>(std::sqrt(reach))
						);
						if (first > last)
						{
							continue;
						}

						const volume_frequency base = {{
							k_y_value * y_step[0],
							k_y_value * y_step[1],
							k_y_value * y_step[2]
						}};
						const auto interval = clip_row(
							base, step, kernel.get_reach(), first, last
						);
						if (interval.is_empty())
						{
							continue;
						}

						const auto stored_row = k_y < 0 ? k_y + signed_n : k_y;
						const virtual_row<T> row(
							negated,
							stored_row,
							k_y_value,
							base,
							step,
							slice + stored_row * slice_core.get_stride(0),
							slice_core.get_stride(1),
							x_phases,
							y_phases[stored_row],
							radial,
							radial_core.get_stride(0),
							last_shell
						);
						kernel.insert(
							row,
							states[index],
							interval.get_begin(),
							interval.get_end()
						);
					}
				}
			}
		}
	}
}

template <typename T, typename SampleWeight>
template <typename U>
inline
void slice_insertion_functor<T, SampleWeight>::copy_tile(
	U *output,
	const operand_core &output_core,
	const U *accumulator,
	const operand_core &accumulator_core,
	const volume_tile &tile
) const noexcept
{
	const auto width = static_cast<std::ptrdiff_t>(output_core.get_extent(2));
	for (auto z = tile.get_z_begin(); z < tile.get_z_end(); ++z)
	{
		for (auto y = tile.get_y_begin(); y < tile.get_y_end(); ++y)
		{
			const auto sz = static_cast<std::ptrdiff_t>(z);
			const auto sy = static_cast<std::ptrdiff_t>(y);
			auto *out = output +
				sz * output_core.get_stride(0) + sy * output_core.get_stride(1);
			const auto *in = accumulator +
				sz * accumulator_core.get_stride(0) +
				sy * accumulator_core.get_stride(1);
			const auto out_x = output_core.get_stride(2);
			const auto in_x = accumulator_core.get_stride(2);
			for (std::ptrdiff_t x = 0; x < width; ++x)
			{
				out[x * out_x] = in[x * in_x];
			}
		}
	}
}

} // namespace cpu
} // namespace rexlib
