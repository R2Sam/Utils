cmake_minimum_required(VERSION 3.28)

# PROJECT_NAME is set by project() below — it is the single place to
project(Utils
	VERSION 0.0.1
	LANGUAGES CXX C
)

# =====================================================================
# Options
# =====================================================================
option(BUILD_LIBRARY  "Build as a library instead of an executable" ON)
option(NATIVE         "Optimise for the host CPU"                  OFF)
option(ENABLE_LTO     "Enable link-time optimisation (portable)"    ON)
option(ENABLE_OPENMP  "Enable OpenMP"                               OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------
# Library type resolution
# ---------------------------------------------------------------------
#   Follows BUILD_SHARED_LIBS (inherited from the parent when fetched).
#   FETCHED_LIB_TYPE overrides the type only for the fetched case. It is a
#   cache variable shared with the other template projects, so setting it
#   once makes Utils and Lua agree.
set(FETCHED_LIB_TYPE "" CACHE STRING
	"Override library type when fetched (empty / STATIC / SHARED)")
set_property(CACHE FETCHED_LIB_TYPE PROPERTY STRINGS "" STATIC SHARED)

if(NOT PROJECT_IS_TOP_LEVEL AND FETCHED_LIB_TYPE)
	set(_lib_type "${FETCHED_LIB_TYPE}")
elseif(BUILD_SHARED_LIBS)
	set(_lib_type SHARED)
else()
	set(_lib_type STATIC)
endif()

# =====================================================================
# Global configuration (top-level only — never leaks to a parent)
# =====================================================================
include(FetchContent)
include(CheckIPOSupported)
include(GenerateExportHeader)

set(FETCHCONTENT_QUIET ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
	set(CMAKE_C_EXTENSIONS OFF)

	if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
	endif()
endif()

# =====================================================================
# Modules
# =====================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/Safety.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Platform.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/CompilerWarnings.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Dependencies.cmake")

# =====================================================================
# Sources
# =====================================================================
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# reflect-cpp (vendored)
set(REFLECT_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/reflect/src/reflectcpp.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/reflect/src/reflectcpp_json.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/reflect/src/yyjson.c"
)

# =====================================================================
# Target
# =====================================================================
if(PROJECT_IS_TOP_LEVEL AND NOT BUILD_LIBRARY)
	# -----------------------------------------------------------------
	# Executable branch (top-level only): builds src/main.cpp as a
	# test / scratch program.
	# -----------------------------------------------------------------
	add_executable(${PROJECT_NAME} ${SRC} ${REFLECT_SRC})

	set_target_properties(${PROJECT_NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	)
else()
	# -----------------------------------------------------------------
	# Library branch.
	# This branch may be fetched into a parent build, so all output
	# must stay under our own binary dir — never the source tree of
	# whoever fetched us.
	# -----------------------------------------------------------------
	list(FILTER SRC EXCLUDE REGEX ".*[/\\\\]main\\.(cpp|cc|cxx|c)$")

	add_library(${PROJECT_NAME} ${_lib_type} ${SRC} ${REFLECT_SRC})
	add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

	# Export macros. BASE_NAME is UTILS (not the template's PROJECT) so the
	# macro (UTILS_EXPORT), the static switch (UTILS_STATIC) and the header
	# name (utils_export.h) cannot collide with a consumer's own
	# PROJECT_EXPORT / project_export.h on a shared include path.
	generate_export_header(${PROJECT_NAME}
		BASE_NAME         UTILS
		EXPORT_MACRO_NAME UTILS_EXPORT
		EXPORT_FILE_NAME  "${CMAKE_CURRENT_BINARY_DIR}/export/utils_export.h"
		STATIC_DEFINE     UTILS_STATIC
	)

	if(_lib_type STREQUAL "SHARED")
		# Deliberately NOT hidden-by-default (unlike the template): reflect-cpp's
		# compiled parts and yyjson are called from header templates in consumers
		# and carry no export annotations.
		if(WIN32)
			set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
		endif()
	else()
		target_compile_definitions(${PROJECT_NAME} PUBLIC UTILS_STATIC)
	endif()

	set_target_properties(${PROJECT_NAME} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/bin"
		ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/bin"
	)
endif()

# Declare C++23 as a public requirement — applies to both branches.
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

# Prevent compiler extensions on this target regardless of the parent's
# globals — the top-level branch above never helps a fetched build.
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_EXTENSIONS OFF
	C_EXTENSIONS   OFF
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/reflect/include>
		# Generated export header (utils_export.h) lives here.
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/reflect/src"
)

project_enable_warnings(${PROJECT_NAME})

# =====================================================================
# Dependencies — add project-specific ones here.
# =====================================================================
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		${PROJECT_NAME}::safety
		${PROJECT_NAME}::platform
)

# libstdc++ experimental library (needed by GCC < 17 for the std features
# Utils uses). Carried over from the previous CMakeLists.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
   AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 17
   AND NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	target_link_libraries(${PROJECT_NAME} PUBLIC stdc++exp)
endif()

# =====================================================================
# LTO (portable)
# =====================================================================
if(ENABLE_LTO)
	check_ipo_supported(RESULT _ipo_ok OUTPUT _ipo_msg LANGUAGES CXX C)

	if(_ipo_ok)
		set_target_properties(${PROJECT_NAME} PROPERTIES
			INTERPROCEDURAL_OPTIMIZATION_RELEASE        ON
			INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON
		)
	else()
		message(STATUS "LTO not supported: ${_ipo_msg}")
	endif()
endif()

# =====================================================================
# OpenMP (opt-in)
# =====================================================================
if(ENABLE_OPENMP)
	find_package(OpenMP REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()
