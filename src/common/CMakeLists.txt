cmake_minimum_required(VERSION 3.9)

set(LIB_NAME challenge_common)

project(${LIB_NAME} VERSION 1.0.1 DESCRIPTION "Common library for Challenge Projects")

add_library(${LIB_NAME} SHARED
	./io/file.cpp
)

set_target_properties(${LIB_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
)

target_include_directories(${LIB_NAME} PRIVATE .)