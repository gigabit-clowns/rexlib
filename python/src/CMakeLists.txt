cmake_minimum_required(VERSION 3.16)

include(FetchContent)
include(GNUInstallDirs)

# Obtain pybind11
FetchContent_Declare(
	pybind11
	GIT_REPOSITORY https://github.com/pybind/pybind11
	GIT_TAG        v2.12.0
)
FetchContent_MakeAvailable(pybind11)

# Register all source and header files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE INLINE_SOURCES ${PROJECT_SOURCE_DIR}/include/*.inl)

# Create the shared library
set(TARGET_NAME "_core_binding")
pybind11_add_module(${TARGET_NAME} ${SOURCES})
set_target_properties(
	${TARGET_NAME}
	PROPERTIES 
		CXX_STANDARD 20
)
target_compile_definitions(
	${TARGET_NAME}
	PRIVATE
		VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
		VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
		VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
)
target_precompile_headers(
	${TARGET_NAME}
	PRIVATE ${HEADERS}
)
target_link_libraries(
	${TARGET_NAME}
	PRIVATE xmipp4-core
)
if(MSVC)
	target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
	target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install library's binary files and headers
install(
	TARGETS ${TARGET_NAME}
	LIBRARY 
	DESTINATION "${PYTHON_INSTALL_DIR}/xmipp4"
)
