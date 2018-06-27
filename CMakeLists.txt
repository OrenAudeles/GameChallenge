cmake_minimum_required(VERSION 3.9)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}/CMakeModules
)

include_directories(
	"${PROJECT_SOURCE_DIR}"
)

add_subdirectory(./src)