// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/error.hpp>

#include <new>
#include <string>

using namespace rexlib;

TEST_CASE(
	"checking a CUDA return code should only throw on failure",
	"[error]"
)
{
	CHECK_NOTHROW( REXLIB_CUDA_CHECK(cudaSuccess) );
	CHECK_THROWS_AS(
		REXLIB_CUDA_CHECK(cudaErrorInvalidValue),
		cuda::error
	);
}

TEST_CASE(
	"a CUDA error should say what failed and where",
	"[error]"
)
{
	try
	{
		REXLIB_CUDA_CHECK(cudaErrorInvalidValue);
		FAIL( "The check should have thrown." );
	}
	catch (const cuda::error &e)
	{
		const std::string message = e.what();

		// Not what the runtime says about the code: it is free to say nothing at
		// all, and does on some platforms.
		CHECK( message.find("cudaErrorInvalidValue") != std::string::npos );
		CHECK( message.find(__FILE__) != std::string::npos );
		CHECK_FALSE( message.empty() );
	}
}

TEST_CASE(
	"checking an allocation should report running out of memory as such",
	"[error]"
)
{
	CHECK_NOTHROW( REXLIB_CUDA_CHECK_ALLOCATION(cudaSuccess) );

	// The allocators answer "no memory left" by asking for less, and tell it
	// from every other failure by its type alone.
	CHECK_THROWS_AS(
		REXLIB_CUDA_CHECK_ALLOCATION(cudaErrorMemoryAllocation),
		std::bad_alloc
	);

	SECTION( "and anything else as an ordinary error" )
	{
		CHECK_THROWS_AS(
			REXLIB_CUDA_CHECK_ALLOCATION(cudaErrorInvalidValue),
			cuda::error
		);

		// Which is not a bad_alloc, so it is not answered by retrying smaller.
		CHECK_THROWS_AS(
			REXLIB_CUDA_CHECK_ALLOCATION(cudaErrorInvalidValue),
			std::runtime_error
		);
	}
}

TEST_CASE(
	"checking a CUDA return code without throwing should never throw",
	"[error]"
)
{
	// Meant for destructors, where there is nowhere left to report a failure
	// to and throwing would end the process.
	CHECK_NOTHROW( REXLIB_CUDA_CHECK_NO_THROW(cudaSuccess) );
	CHECK_NOTHROW( REXLIB_CUDA_CHECK_NO_THROW(cudaErrorInvalidValue) );
	CHECK_NOTHROW( REXLIB_CUDA_CHECK_NO_THROW(cudaErrorMemoryAllocation) );
}

TEST_CASE(
	"a CUDA error should describe a code the runtime says nothing about",
	"[error]"
)
{
	// The runtime says nothing at all about some codes, and streaming that
	// straight into the message reads past a null pointer.
	const auto unrecognized = static_cast<cudaError_t>(0x7ffffff0);

	try
	{
		REXLIB_CUDA_CHECK(unrecognized);
		FAIL( "The check should have thrown." );
	}
	catch (const cuda::error &e)
	{
		// Whatever the runtime had to say about it, the message still says
		// which call failed and where.
		const std::string message = e.what();
		CHECK( message.find("unrecognized") != std::string::npos );
		CHECK( message.find(__FILE__) != std::string::npos );
	}

	CHECK_NOTHROW( REXLIB_CUDA_CHECK_NO_THROW(unrecognized) );
}
