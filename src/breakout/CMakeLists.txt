cmake_minimum_required(VERSION 3.9)

set(APPNAME breakout)

set(PROJ ${APPNAME})
set(PROJ_DEBUG ${APPNAME}_debug)

project(${PROJ})
project(${PROJ_DEBUG})

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

set(SOURCES
    main.cpp
)
set(ALL_SOURCES
    ${SOURCES}
)

add_executable(${PROJ} ${ALL_SOURCES})
add_executable(${PROJ_DEBUG} ${ALL_SOURCES})

message(STATUS "SDL2 Lib Path: ${SDL2_LIBRARY}")
set(PROJ_LIBS
   ${SDL2_LIBRARY}
   ${SDL2_IMAGE_LIBRARIES}
   ${OPENGL_LIBRARY}

   GLEW
   dl
)
set(PROJ_DEBUG_LIBS
    ${PROJ_LIBS}
)
# Shouldn't require explicit dependency with OpenGL and DevIL
target_link_libraries(${PROJ} ${PROJ_LIBS})
target_link_libraries(${PROJ_DEBUG} ${PROJ_DEBUG_LIBS})

add_custom_command(
    TARGET ${PROJ} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROJ}${CMAKE_EXECUTABLE_SUFFIX}"
    "${OUTPUT_DIR}/"
)

add_custom_command(
    TARGET ${PROJ_DEBUG} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROJ_DEBUG}${CMAKE_EXECUTABLE_SUFFIX}"
    "${OUTPUT_DIR}/"
)

if (CMAKE_COMPILER_IS_GNUCC)
# PROJ
    set_property( TARGET ${PROJ} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wall")
    set_property( TARGET ${PROJ} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wpedantic")
    set_property( TARGET ${PROJ} APPEND_STRING PROPERTY COMPILE_FLAGS " -O2")
    set_property( TARGET ${PROJ} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-exceptions")
    set_property( TARGET ${PROJ} APPEND_STRING PROPERTY COMPILE_FLAGS " -Werror")

    set_property( TARGET ${PROJ} APPEND_STRING PROPERTY LINK_FLAGS " -no-pie")

# PROJ_DEBUG
    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wall")
    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wpedantic")
    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY COMPILE_FLAGS " -O2")
    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY COMPILE_FLAGS " -g")
    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-exceptions")
    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY COMPILE_FLAGS " -Werror")

    set_property( TARGET ${PROJ_DEBUG} APPEND_STRING PROPERTY LINK_FLAGS " -no-pie")
endif (CMAKE_COMPILER_IS_GNUCC)