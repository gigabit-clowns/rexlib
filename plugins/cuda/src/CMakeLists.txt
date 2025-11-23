cmake_minimum_required(VERSION 3.18)

# Find all source and header files
file(
	GLOB_RECURSE 
	SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/*.cu
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
		${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# Create the shared library
set(TARGET_NAME ${PROJECT_NAME})
add_library(${TARGET_NAME} SHARED ${SOURCES})
set_target_properties(
	${TARGET_NAME} 
	PROPERTIES 
		CXX_STANDARD 20
		DEFINE_SYMBOL "XMIPP4_CUDA_EXPORTING"
)
target_include_directories(
	${TARGET_NAME} 
	PUBLIC 
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(
	${TARGET_NAME} 
	PRIVATE
		VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
		VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
		VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
)
target_link_libraries(
	${TARGET_NAME} 
	PUBLIC
		xmipp4-core
		CUDA::cudart
)

# Enable all warnings during compilation
if(MSVC)
	target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
	target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Disable min/max macros in windows
if(MSVC)
	target_compile_definitions(${TARGET_NAME} PUBLIC -DNOMINMAX)
endif()

# Install library's binary files and headers
install(
	TARGETS ${TARGET_NAME}
	EXPORT ${TARGET_NAME}-config
	LIBRARY DESTINATION ${XMIPP4_PLUGIN_INSTALL_DIR}
	RUNTIME DESTINATION ${XMIPP4_PLUGIN_INSTALL_DIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/ 
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
  	EXPORT ${TARGET_NAME}-config
  	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET_NAME}
)
