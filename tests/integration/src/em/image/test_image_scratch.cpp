// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/image/buffer_image_scratch.hpp>
#include <rexlib/em/image/scratch_image_reader_provider.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"
#include "fixtures/scoped_path.hpp"

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/mapped_file_buffer.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/layout/index_table.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/direct_image_reader_provider.hpp>
#include <rexlib/em/image/executor_image_loader.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_read.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>
#include <rexlib/em/image/image_write.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/functional/creation.hpp>

#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// Every stack holds six images of 8 by 8.
const std::size_t stack_count = 3;
const std::size_t image_count = 6;
const std::size_t image_extent = 8;
const std::size_t image_size = image_extent * image_extent;
const std::size_t image_bytes = image_size * sizeof(float);

// Room for every image of the three stacks.
const std::size_t dataset_bytes = stack_count * image_count * image_bytes;

const float untouched = -1.0F;

// Three stacks on disk whose every value tells its stack, its image and its
// place in the image, and the provider that reads them as they are.
class image_scratch_fixture
	: public cpu_execution_context_fixture
{
public:
	image_scratch_fixture()
		: m_first("image_scratch_stack_0.mrcs")
		, m_second("image_scratch_stack_1.mrcs")
		, m_third("image_scratch_stack_2.mrcs")
		, m_storage_file("image_scratch_storage.mapped")
	{
		for (std::size_t stack = 0; stack < stack_count; ++stack)
		{
			write_stack_file(stack);
		}

		direct = std::make_shared<direct_image_reader_provider>(
			catalog.get_service_manager<image_read_format_manager>()
		);
	}

protected:
	const std::string& get_path(std::size_t stack) const noexcept
	{
		const scoped_path *paths[stack_count] = {
			&m_first,
			&m_second,
			&m_third
		};

		return paths[stack]->get();
	}

	image_location locate(std::size_t stack, std::size_t image) const
	{
		return image_location(get_path(stack), image);
	}

	// A buffer in main memory, or one that is a mapped file.
	std::shared_ptr<buffer> make_storage(bool in_a_file, std::size_t size) const
	{
		if (in_a_file)
		{
			return create_mapped_file_buffer(m_storage_file.get(), size);
		}

		const auto allocator = get_host_memory_resource().create_allocator();
		return allocator->allocate(size, sizeof(float));
	}

	// A provider that reads the stacks through a scratch of some locations.
	std::shared_ptr<image_reader_provider> make_scratched(
		const std::vector<image_location> &held,
		std::shared_ptr<buffer> storage,
		std::size_t run_size = std::numeric_limits<std::size_t>::max()
	) const
	{
		const auto scratch = std::make_shared<buffer_image_scratch>(
			make_span(held),
			*direct,
			std::move(storage),
			run_size
		);

		return std::make_shared<scratch_image_reader_provider>(
			direct,
			scratch
		);
	}

	std::vector<float> read_batch(
		const std::shared_ptr<image_reader_provider> &readers,
		const std::vector<image_location> &locations
	) const
	{
		const executor_image_loader loader(
			readers,
			std::make_shared<synchronous_executor>()
		);
		const std::vector<std::size_t> extents = {
			locations.size(), image_extent, image_extent
		};
		auto destination = full(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			untouched,
			context
		);

		const auto completion = read_batch_async(
			loader,
			destination.share(),
			make_span(locations)
		);
		REQUIRE_NOTHROW( completion->get() );

		return read_host<float>(destination, locations.size() * image_size);
	}

	std::shared_ptr<image_reader_provider> direct;

private:
	void write_stack_file(std::size_t stack)
	{
		const std::vector<std::size_t> extents = {
			image_count, image_extent, image_extent
		};
		auto values = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);

		auto *data = static_cast<float*>(values.get_storage()->get_host_ptr());
		for (std::size_t i = 0; i < image_count * image_size; ++i)
		{
			data[i] = static_cast<float>(1000 * stack + i);
		}

		write_stack(
			const_array_ref(values),
			get_path(stack),
			*catalog.get_service_manager<image_write_format_manager>()
		);
	}

	scoped_path m_first;
	scoped_path m_second;
	scoped_path m_third;
	scoped_path m_storage_file;
};

} // anonymous namespace

TEST_CASE_METHOD( image_scratch_fixture,
	"a batch read through a scratch holds what the files hold",
	"[image_scratch]" )
{
	const auto in_a_file = GENERATE(false, true);

	// A batch drawn at random from the three stacks.
	const std::vector<image_location> batch = {
		locate(2, 4), locate(0, 1), locate(1, 5), locate(0, 3),
		locate(2, 0), locate(1, 2), locate(0, 5), locate(2, 2)
	};
	const auto expected = read_batch(direct, batch);

	SECTION( "when the scratch holds every image of the batch" )
	{
		const auto scratched = make_scratched(
			batch,
			make_storage(in_a_file, dataset_bytes)
		);

		// The first pass loads the scratch and the second only reads it.
		CHECK( read_batch(scratched, batch) == expected );
		CHECK( read_batch(scratched, batch) == expected );
	}

	SECTION( "when its buffer has room for only some of them" )
	{
		const auto scratched = make_scratched(
			batch,
			make_storage(in_a_file, 3 * image_bytes)
		);

		CHECK( read_batch(scratched, batch) == expected );
		CHECK( read_batch(scratched, batch) == expected );
	}

	SECTION( "when it loads a run of two images at a time" )
	{
		const auto scratched = make_scratched(
			batch,
			make_storage(in_a_file, dataset_bytes),
			2 * image_bytes
		);

		CHECK( read_batch(scratched, batch) == expected );
		CHECK( read_batch(scratched, batch) == expected );
	}

	SECTION( "when it holds other images than those of the batch" )
	{
		const std::vector<image_location> held = {
			locate(0, 0), locate(0, 1), locate(1, 4), locate(2, 2)
		};
		const auto scratched = make_scratched(
			held,
			make_storage(in_a_file, dataset_bytes)
		);

		CHECK( read_batch(scratched, batch) == expected );
		CHECK( read_batch(scratched, batch) == expected );
	}
}

TEST_CASE_METHOD( image_scratch_fixture,
	"patches read through a scratch hold what the file holds",
	"[image_scratch]" )
{
	const auto in_a_file = GENERATE(false, true);

	// One patch inside the image, and one over a corner of it.
	const std::size_t patch_extent = 4;
	const auto location = locate(1, 2);
	index_table centres(2);
	const std::size_t inside[2] = {4, 4};
	const std::size_t corner[2] = {0, 0};
	centres.add(make_span(inside, 2));
	centres.add(make_span(corner, 2));

	const auto read_patches = [&] (
		const std::shared_ptr<image_reader_provider> &readers
	)
	{
		const executor_image_loader loader(
			readers,
			std::make_shared<synchronous_executor>()
		);
		const std::vector<std::size_t> extents = {
			2, patch_extent, patch_extent
		};
		auto destination = full(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			untouched,
			context
		);

		const auto completion = read_patches_async(
			loader,
			destination.share(),
			location,
			centres
		);
		REQUIRE_NOTHROW( completion->get() );

		return read_host<float>(destination, 2 * patch_extent * patch_extent);
	};

	const auto expected = read_patches(direct);
	const auto scratched = make_scratched(
		{location, locate(1, 3)},
		make_storage(in_a_file, dataset_bytes)
	);

	CHECK( read_patches(scratched) == expected );
	CHECK( read_patches(scratched) == expected );
}

TEST_CASE_METHOD( image_scratch_fixture,
	"a whole file read through a scratch holds what the file holds",
	"[image_scratch]" )
{
	const auto in_a_file = GENERATE(false, true);

	const image_location whole(get_path(1));
	const auto element_count = image_count * image_size;
	const auto expected = read_host<float>(
		em::read(whole, *direct, context),
		element_count
	);

	SECTION( "when the scratch holds the whole file" )
	{
		const auto scratched = make_scratched(
			{whole},
			make_storage(in_a_file, dataset_bytes)
		);

		CHECK( read_host<float>(
			em::read(whole, *scratched, context),
			element_count
		) == expected );
		CHECK( read_host<float>(
			em::read(whole, *scratched, context),
			element_count
		) == expected );
	}

	SECTION( "when the scratch holds some of its images" )
	{
		const auto scratched = make_scratched(
			{locate(1, 0), locate(1, 4)},
			make_storage(in_a_file, dataset_bytes)
		);

		CHECK( read_host<float>(
			em::read(whole, *scratched, context),
			element_count
		) == expected );
	}
}

TEST_CASE_METHOD( image_scratch_fixture,
	"an image read through a scratch is converted as the file would be",
	"[image_scratch]" )
{
	const auto in_a_file = GENERATE(false, true);

	const auto location = locate(2, 3);
	const auto expected = read_host<double>(
		em::read(location, *direct, context, numerical_type::float64),
		image_size
	);
	const auto scratched = make_scratched(
		{location},
		make_storage(in_a_file, dataset_bytes)
	);

	CHECK( read_host<double>(
		em::read(location, *scratched, context, numerical_type::float64),
		image_size
	) == expected );
	CHECK( read_host<double>(
		em::read(location, *scratched, context, numerical_type::float64),
		image_size
	) == expected );
}
