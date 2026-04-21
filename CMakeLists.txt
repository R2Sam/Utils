cmake_minimum_required(VERSION 3.25)

project(
	Utils
	VERSION 0.0.1
	LANGUAGES CXX
)

# -------------------------
# Global configuration
# -------------------------
include(FetchContent)

set(FETCHCONTENT_QUIET ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Detect Emscripten
set(IS_EMSCRIPTEN OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set(IS_EMSCRIPTEN ON)
endif()

# -------------------------
# Options
# -------------------------
add_library(Utils_options INTERFACE)
add_library(Utils::options ALIAS Utils_options)

target_compile_features(
	Utils_options
	INTERFACE
		cxx_std_23
)

# -------------------------
# Safety
# -------------------------
add_library(Utils_safety INTERFACE)
add_library(Utils::safety ALIAS Utils_safety)

target_compile_options(
	Utils_safety
	INTERFACE
		-fstack-protector-strong
		-D_GLIBCXX_ASSERTIONS

		$<$<CONFIG:Debug>:
			-O0
			-g3
			-gdwarf-4
			-fno-inline
			-fno-omit-frame-pointer

			$<$<AND:$<PLATFORM_ID:Linux>,$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>>:
                -fsanitize=address
                -fsanitize=undefined
            >
		>

		$<$<CONFIG:Release>:
            -O3

            $<$<AND:$<PLATFORM_ID:Linux>,$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>>:
                -fsanitize-trap=undefined
            >
        >
)

target_link_options(
	Utils_safety
	INTERFACE
		$<$<AND:$<CONFIG:Debug>,$<PLATFORM_ID:Linux>,$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>>:
			-fsanitize=address
			-fsanitize=undefined
		>

		$<$<AND:$<CONFIG:Release>,$<PLATFORM_ID:Linux>,$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>>:
		>
)

# -------------------------
# Platform
# -------------------------
add_library(Utils_platform INTERFACE)
add_library(Utils::platform ALIAS Utils_platform)

target_link_libraries(
	Utils_platform
	INTERFACE
		$<$<PLATFORM_ID:Windows>:
		>
)

target_link_options(
	Utils_platform
	INTERFACE
		$<$<PLATFORM_ID:Windows>:
			-static
		>
)

target_link_libraries(
	Utils_platform
	INTERFACE
		$<$<AND:$<PLATFORM_ID:Linux>,$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>>:
		>
)

# -------------------------
# Utils static library
# -------------------------
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp)
list(FILTER SRC EXCLUDE REGEX ".*/main\\.cpp$")

add_library(
	utils 
	STATIC
		${SRC}
		reflect/src/reflectcpp.cpp
		reflect/src/reflectcpp_json.cpp
		reflect/src/yyjson.c
)

target_include_directories(
	utils
	PUBLIC
		include
		src
		reflect/include
	PRIVATE
		reflect/src
)

target_compile_options(
	utils
	PRIVATE
		-Wall
		-Wextra
		-pedantic
		-fmax-errors=5

		-Wno-missing-field-initializers
		-Wno-narrowing
		-Wno-enum-compare
		-Wno-reorder
		-Wno-shadow
		-Wno-deprecated-declarations
)

target_link_libraries(
	utils
	PRIVATE
		Utils::options
		Utils::safety
		Utils::platform

	PUBLIC
		stdc++exp
)

# -------------------------
# Emscripten
# -------------------------
target_compile_options(
	utils
	PRIVATE
		$<$<BOOL:${IS_EMSCRIPTEN}>:
		>
)

target_link_options(
	utils
	PRIVATE
		$<$<BOOL:${IS_EMSCRIPTEN}>:
		>
)