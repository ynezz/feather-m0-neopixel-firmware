cmake_minimum_required(VERSION 3.9)

set(CMAKE_MODULE_PATH
	${CMAKE_SOURCE_DIR}/cmake
	${CMAKE_SOURCE_DIR}/cmake/target-boards
	${CMAKE_SOURCE_DIR}/cmake/modules
)

include(config.cmake)
include(clang-format)

project(feather-m0-neopixel-firmware C CXX ASM)
add_sketches(sketches)
