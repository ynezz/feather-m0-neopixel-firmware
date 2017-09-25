# Cortex-M0/Arduino CMake toolchain
#
# Adds the toolchain and Arduino IDE libs
#
# @author Matthias L. Jugel <leo@ubirch.com>
# @author Petr Stetiar <ynezz@true.cz>
#
# == LICENSE ==
# Copyright 2015 ubirch GmbH
# Copyright 2017 Petr Stetiar
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Feel free to use whatever version works for you, but remember, that I'm
# myself using same version and same version is being used on CircleCI, so in
# other words, if you change this version, please don't waste my time asking
# for help :-)
cmake_minimum_required(VERSION 3.9)

set(BIN_PREFIX arm-none-eabi-)
find_program(SAMDCPP ${BIN_PREFIX}g++)
find_program(SAMDC ${BIN_PREFIX}gcc)
find_program(SAMDAR ${BIN_PREFIX}ar)
find_program(SAMDSTRIP ${BIN_PREFIX}strip)
find_program(OBJCOPY ${BIN_PREFIX}objcopy)
find_program(OBJDUMP ${BIN_PREFIX}objdump)
find_program(SAMDSIZE ${BIN_PREFIX}size)
find_program(BOSSAC bossac PATHS "${ARDUINO_BOSSAC_PATH}")
find_program(SCREEN screen)

# toolchain settings
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER ${SAMDCPP})
set(CMAKE_C_COMPILER ${SAMDC})
set(CMAKE_ASM_COMPILER ${SAMDC})

# Important project paths
set(BASE_PATH "${${PROJECT_NAME}_SOURCE_DIR}")
set(SRC_PATH "${BASE_PATH}/src")
set(LIB_PATH "${BASE_PATH}/lib")

if (NOT DEFINED MONITOR_BAUD)
	set(MONITOR_BAUD 115200)
endif()
if (NOT DEFINED MONITOR_CMD)
	set(MONITOR_CMD ${SCREEN})
endif()
if (NOT DEFINED MONITOR_DEV)
	set(MONITOR_DEV ${SERIAL_DEV})
endif()
if (NOT DEFINED MONITOR_ARGS)
	set(MONITOR_ARGS ${MONITOR_DEV} ${MONITOR_BAUD})
endif()
if (NOT DEFINED BOSSAC_ARGS)
	set(BOSSAC_ARGS -i -d --port=${SERIAL_DEV} -U true -i -e -w -v -R)
endif()
if (NOT DEFINED BOARD_RESET_COMMAND)
	set(BOARD_RESET_COMMAND stty -F ${SERIAL_DEV} 1200 && sleep 2)
endif()
if (NOT DEFINED COMPILER_WARNING_FLAGS)
	set(COMPILER_WARNING_FLAGS "-w -Wall -Werror -Wextra")
endif()
if (NOT DEFINED COMPILER_C_STANDARD)
	set(COMPILER_C_STANDARD "gnu11")
endif()
if (NOT DEFINED COMPILER_CXX_STANDARD)
	set(COMPILER_CXX_STANDARD "gnu++11")
endif()
if (NOT DEFINED ARDUINO_VERSION_CFLAGS)
	set(ARDUINO_VERSION_CFLAGS "10803")
endif()
add_definitions(-DARDUINO=${ARDUINO_VERSION_CFLAGS})

message(STATUS "Configured for ${TARGET_BOARD}, using serial port ${SERIAL_DEV}")

add_definitions(-nostdlib)
set(COMPILER_FLAGS "${COMPILER_BOARD_FLAGS} -g -Os ${COMPILER_WARNING_FLAGS} -ffunction-sections -fdata-sections --param max-inline-insns-single=500 -MMD" CACHE STRING "")
set(CMAKE_C_FLAGS "${COMPILER_FLAGS} -std=${COMPILER_C_STANDARD}" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${COMPILER_FLAGS} -std=${COMPILER_CXX_STANDARD} -fno-rtti -fno-exceptions" CACHE STRING "")
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp ${COMPILER_FLAGS}" CACHE STRING "")
set(CMAKE_LNK_FLAGS "--specs=nano.specs --specs=nosys.specs -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -save-temps -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LNK_FLAGS} ${EXTRA_LINKER_FLAGS} ${LINKER_BOARD_FLAGS}" CACHE STRING "")

# we need a little function to add multiple targets
function(add_executable_samd NAME)
	if (DEFINED MCU_REQUIRED AND (NOT (MCU STREQUAL "${MCU_REQUIRED}")))
		message(STATUS "Ignoring target ${NAME} (required MCU '${MCU_REQUIRED}' != '${MCU}')")
	else ()
		add_executable(${NAME} ${ARGN})
		set(LINKER_SCRIPT "${ADAFRUIT_SAMD_PATH}/variants/arduino_zero/linker_scripts/gcc/flash_with_bootloader.ld")
		set_target_properties(${NAME} PROPERTIES OUTPUT_NAME "${NAME}.elf")
		set_target_properties(${NAME} PROPERTIES LINK_FLAGS "\"-Wl,-Map,${NAME}.map\" \"-T${LINKER_SCRIPT}\" -lm \"-L${ARDUINO_CMSIS_LIB_PATH}\" -larm_cortexM0l_math")
		set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${NAME}.hex;${NAME}.eep;${NAME}.lst")

		# generate the .hex file
		add_custom_command(
			OUTPUT ${NAME}.hex
			COMMAND ${SAMDSTRIP} "${NAME}.elf"
			COMMAND ${SAMDSIZE} "${NAME}.elf"
			COMMAND ${OBJCOPY} -O binary "${NAME}.elf" "${NAME}.bin"
			COMMAND ${OBJCOPY} -O ihex -R .eeprom "${NAME}.elf" "${NAME}.hex"
			DEPENDS ${NAME})

		add_custom_target(${NAME}-reset
			COMMAND ${BOARD_RESET_COMMAND}
			DEPENDS ${NAME}.hex)

		add_custom_target(
			${NAME}-flash
			COMMAND ${BOSSAC} ${BOSSAC_ARGS} ${NAME}.bin
			DEPENDS ${NAME}.hex ${NAME}-reset)

		add_custom_target(
			${NAME}-monitor
			COMMAND sleep 1 && ${MONITOR_CMD} ${MONITOR_ARGS})
	endif ()
endfunction(add_executable_samd)

# add all the libraries as possible library dependencies
function(add_libraries LIB_PATH)
	set(PROJECT_LIBS)
	file(GLOB LIB_DIRS "${LIB_PATH}/*/CMakeLists.txt")
	foreach (cmakedir ${LIB_DIRS})
		get_filename_component(subdir ${cmakedir} PATH)
		if (IS_DIRECTORY ${subdir})
			get_filename_component(target ${subdir} NAME)
			message(STATUS "Adding SAMD library: ${target}")
			add_subdirectory(${subdir})
		endif ()
	endforeach ()
endfunction()

# add targets automatically from the src directory
function(add_sources SRC_PATH)
	file(GLOB SRC_DIRS "${SRC_PATH}/*/CMakeLists.txt")
	foreach (cmakedir ${SRC_DIRS})
		get_filename_component(subdir ${cmakedir} PATH)
		if (IS_DIRECTORY ${subdir})
			get_filename_component(target ${subdir} NAME)
			message(STATUS "Found SAMD target: ${target}")
			add_subdirectory(${subdir})
		endif ()
	endforeach ()
endfunction()


# ====================================================================================================================
# ARDUINO SKETCH FUNCTIONALITY
# ====================================================================================================================

set(ARDUINO_SAMD_PATH ${ADAFRUIT_SAMD_PATH})
set(ARDUINO_CORE_LIBS "" CACHE INTERNAL "arduino core libraries")
function(setup_arduino_core)
	if (DEFINED ARDUINO_SAMD_PATH AND IS_DIRECTORY ${ARDUINO_SAMD_PATH} AND NOT (TARGET arduino-core))
		# set the paths
		set(ARDUINO_CORES_PATH ${ARDUINO_SAMD_PATH}/cores/arduino CACHE STRING "")
		set(ARDUINO_LIBRARIES_PATH ${ARDUINO_SAMD_PATH}/libraries CACHE STRING "")
		set(ARDUINO_VARIANT_PATH ${ARDUINO_SAMD_PATH}/variants/${BOARD_VARIANT} CACHE STRING "")

		file(GLOB_RECURSE CORE_SOURCES
				${ARDUINO_CORES_PATH}/*.S ${ARDUINO_CORES_PATH}/*.c ${ARDUINO_CORES_PATH}/*.cpp
				${ARDUINO_VARIANT_PATH}/*.S ${ARDUINO_VARIANT_PATH}/*.c ${ARDUINO_VARIANT_PATH}/*.cpp)

		set(ARDUINO_CORE_SRCS ${CORE_SOURCES} CACHE STRING "Arduino core library sources")

		# setup the main arduino core target
		add_library(arduino-core ${ARDUINO_CORE_SRCS})
		# target_link_libraries(arduino-core m arm_cortexM0l_math)
		# link_directories(${ARDUINO_CMSIS_LIB_PATH})
		target_include_directories(arduino-core PUBLIC ${ARDUINO_CORES_PATH})
		target_include_directories(arduino-core PUBLIC ${ARDUINO_VARIANT_PATH})
		target_include_directories(arduino-core PUBLIC ${ARDUINO_CMSIS_INCLUDE_PATH})
		target_include_directories(arduino-core PUBLIC ${ARDUINO_CMSIS_ATMEL_INCLUDE_PATH})

		# configure all the additional libraries in the core
		file(GLOB CORE_DIRS ${ARDUINO_LIBRARIES_PATH}/*)
		foreach (libdir ${CORE_DIRS})
			get_filename_component(libname ${libdir} NAME)
			if (IS_DIRECTORY ${libdir})
				file(GLOB_RECURSE sources ${libdir}/*.cpp ${libdir}/*.S ${libdir}/*.c)
				string(REGEX REPLACE "examples?/.*" "" sources "${sources}")
				if (sources)
					if (NOT TARGET ${libname})
						message(STATUS "Adding Arduino Core library: ${libname}")
						add_library(${libname} ${sources})
					endif ()
					target_link_libraries(${libname} arduino-core)
					foreach (src ${sources})
						get_filename_component(dir ${src} PATH)
						target_include_directories(${libname} PUBLIC ${dir})
					endforeach ()
					list(APPEND ARDUINO_CORE_LIBS ${libname})
					set(ARDUINO_CORE_LIBS ${ARDUINO_CORE_LIBS} CACHE INTERNAL "arduino core libraries")
				else ()
					include_directories(SYSTEM ${libdir})
				endif ()
			endif ()
		endforeach ()
	endif ()
endfunction()

function(add_sketches SKETCHES_PATH)
	setup_arduino_core()
	if (TARGET arduino-core)
		# finally add all the sketches
		file(GLOB SRC_DIRS "${SKETCHES_PATH}/*/CMakeLists.txt")
		foreach (cmakedir ${SRC_DIRS})
			get_filename_component(subdir ${cmakedir} PATH)
			if (IS_DIRECTORY ${subdir})
				get_filename_component(target ${subdir} NAME)
				message(STATUS "Found Arduino sketch: ${target}")
				add_subdirectory(${subdir})
			endif ()
		endforeach ()
	else ()
		message(WARNING "No Arduino SAMD platfrom found at '${ARDUINO_SAMD_PATH}', can't compile sketches!")
	endif ()
endfunction()

set(SKETCH_LIBS "" CACHE INTERNAL "arduino sketch libraries")
# add a sketch dependency by giving the target and a git url
function(target_sketch_library TARGET NAME URL)
	setup_arduino_core()
	if (TARGET arduino-core AND NOT TARGET ${NAME})
		# try to install dependent libraries
		find_package(Git)
		if (GIT_FOUND)
			# clone dependency into libraries
			get_filename_component(MYLIBS "${CMAKE_CURRENT_SOURCE_DIR}/../libraries" REALPATH)
			if (NOT EXISTS ${MYLIBS})
				message(STATUS "Creating library directory: ${MYLIBS}")
				file(MAKE_DIRECTORY ${MYLIBS})
			else()
				message(STATUS "Using existing sketches library directory: ${MYLIBS}")
			endif ()
			if (NOT EXISTS ${MYLIBS}/${NAME})
				message(STATUS "Installing ${NAME} (${URL})")
				execute_process(
					COMMAND ${GIT_EXECUTABLE} clone "${URL}" "${NAME}"
					WORKING_DIRECTORY ${MYLIBS})
			endif ()

			# now add a library target
			file(GLOB_RECURSE libsources FOLLOW_SYMLINKS ${MYLIBS}/${NAME}/*.S ${MYLIBS}/${NAME}/*.c ${MYLIBS}/${NAME}/*.cpp)
			# go through list of sources and remove the examples
			foreach (file ${libsources})
				STRING(REGEX MATCH "/examples?/|/tests?/" example ${file})
				if (NOT example)
					list(APPEND sources ${file})
				endif ()
			endforeach ()

			if (sources)
				message(STATUS "Adding external library: ${NAME}")
				add_library(${NAME} ${sources})
				target_link_libraries(${NAME} ${ARDUINO_CORE_LIBS} ${SKETCH_LIBS})
				list(APPEND SKETCH_LIBS ${NAME})
				set(SKETCH_LIBS ${SKETCH_LIBS} CACHE INTERNAL "arduino sketch libraries")
				foreach (src ${sources})
					get_filename_component(dir ${src} PATH)
					list(APPEND includes ${dir})
				endforeach ()
				list(REMOVE_DUPLICATES includes)
				target_include_directories(${NAME} PUBLIC ${includes})
				target_link_libraries(${TARGET} ${NAME})
			else ()
				include_directories(SYSTEM ${MYLIBS}/${NAME})
			endif ()
		else ()
			message(FATAL_ERROR "Missing git, please install and try again!")
		endif ()
	else ()
		target_link_libraries(${TARGET} ${NAME})
	endif ()
endfunction(target_sketch_library)
