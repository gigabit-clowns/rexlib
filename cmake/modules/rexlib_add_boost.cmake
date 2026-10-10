include(FetchContent)

# Makes the Boost modules this project uses available as Boost:: targets,
# either from the system or by fetching and building them.
#
# VERSION is the release to fetch and MINIMUM the one find_package() demands.
# They are separate because the fetched pin tracks the newest release, which
# is a floor no distribution would meet.
function(rexlib_add_boost)
	set(options)
	set(oneValueArgs VERSION MINIMUM)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	if(REXLIB_USE_SYSTEM_BOOST)
		# CONFIG mode, because only BoostConfig.cmake, which Boost has
		# installed since 1.70, names the modules individually.
		find_package(
			Boost ${arg_MINIMUM} REQUIRED CONFIG
			COMPONENTS filesystem
		)

		# A Boost installed by b2 defines targets for its compiled
		# libraries and a single Boost::headers for everything else,
		# whereas the fetched build defines one target per module. Fill
		# in the header only ones so that the names this project links
		# do not depend on where Boost came from.
		foreach(module container container_hash)
			if(NOT TARGET Boost::${module})
				add_library(Boost::${module} INTERFACE IMPORTED)
				set_target_properties(
					Boost::${module}
					PROPERTIES
						INTERFACE_LINK_LIBRARIES Boost::headers
				)
			endif()
		endforeach()

		return()
	endif()

	if(POLICY CMP0135)
		cmake_policy(SET CMP0135 NEW) # To avoid warnings
	endif()

	set(archive "https://github.com/boostorg/boost/releases/download")
	set(url "${archive}/boost-${arg_VERSION}/boost-${arg_VERSION}-cmake.tar.gz")
	FetchContent_Declare(Boost URL "${url}")

	# Only the modules the sources include. Every one listed here is built,
	# so an unused entry is paid for on every clean build.
	set(BOOST_INCLUDE_LIBRARIES
		container
		container_hash
		filesystem
	)
	set(BOOST_CONTAINER_HEADER_ONLY ON)
	set(BOOST_USE_STATIC_LIBS ON)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	FetchContent_MakeAvailable(Boost)
endfunction()
