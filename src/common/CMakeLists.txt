cmake_minimum_required(VERSION 3.9)

set(LIB_NAME challenge_common)

project(${LIB_NAME} VERSION 1.0.1 DESCRIPTION "Common library for Challenge Projects")

message(STATUS "Searching for SDL2")
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

if (NOT SDL2_FOUND)
    message(FATAL_ERROR "This project requires SDL2 to be installed to be compiled")
endif (NOT SDL2_FOUND)

if (NOT SDL2_IMAGE_FOUND)

endif (NOT SDL2_IMAGE_FOUND)

message(STATUS "SDL2 Include Dir: ${SDL2_INCLUDE_DIR}")

find_package(OpenGL REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    .
)

add_definitions( -DGLEW_STATIC)

set(PROJ_LIBS
   ${SDL2_LIBRARY}
   ${SDL2_IMAGE_LIBRARIES}
   ${OPENGL_LIBRARY}

   GLEW
)

add_library(${LIB_NAME} SHARED
	./api.cpp

	./io/file.cpp

	./graphics/image_SDL.cpp
	./graphics/shader.c
	./graphics/texture.c
	./graphics/renderer.cpp

	./event/event_handler.cpp
	./event/event_SDL.cpp
	
)

set_target_properties(${LIB_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
)

target_include_directories(${LIB_NAME} PRIVATE .)
target_link_libraries(${LIB_NAME} ${PROJ_LIBS})


if (CMAKE_COMPILER_IS_GNUCC)
# PROJ
    set_property( TARGET ${LIB_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wall")
    set_property( TARGET ${LIB_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wpedantic")
    set_property( TARGET ${LIB_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS " -O2")
    set_property( TARGET ${LIB_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-exceptions")
    set_property( TARGET ${LIB_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS " -Werror")
endif (CMAKE_COMPILER_IS_GNUCC)