cmake_minimum_required(VERSION 3.25)

project(Utils
  VERSION 1.0.0
  LANGUAGES C CXX
)

# ------------------------------------------------------------------------------
# C++ standard & global settings
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# ------------------------------------------------------------------------------
# Platform detection
# ------------------------------------------------------------------------------
if (WIN32)
  set(SYSTEM_LIBS
    opengl32
    gdi32
    winmm
    ws2_32
    gomp
    -static
  )

  set(RELEASE_FLAGS        -O3)
  set(DEBUG_FLAGS          -Og -gdwarf-4)
  set(RELEASE_LINKER_FLAGS -s -mwindows)

elseif (UNIX AND NOT APPLE AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
  set(SYSTEM_LIBS
    GL
    m
    pthread
    dl
    rt
    X11
    gomp
  )

  set(RELEASE_FLAGS        -O3)
  set(DEBUG_FLAGS          -Og -gdwarf-4 -fno-omit-frame-pointer)
  set(RELEASE_LINKER_FLAGS -s)

elseif (UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
  message(FATAL_ERROR "Linux ARM is not supported")

else()
  message(FATAL_ERROR "Unsupported platform")
endif()

# ------------------------------------------------------------------------------
# Common compiler warnings & flags
# ------------------------------------------------------------------------------
set(COMMON_FLAGS
  -Wall
  -Wextra
  -pedantic
  -fopenmp
  -fmax-errors=5
  -Wno-missing-field-initializers
  -Wno-narrowing
  -Wno-enum-compare
  -Wno-reorder
  -Wno-shadow
  -Wno-deprecated-declarations
)

# ------------------------------------------------------------------------------
# Utils static library
# ------------------------------------------------------------------------------
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
  src/*.cpp
  src/*.hpp
)

list(FILTER SRC EXCLUDE REGEX ".*/main\\.cpp$")

add_library(utils
  ${SRC}
  reflect/src/reflectcpp.cpp
  reflect/src/reflectcpp_json.cpp
  reflect/src/yyjson.c)

target_include_directories(utils
  PUBLIC
    src
    include
    reflect/src
    reflect/include
)

target_compile_options(utils PRIVATE
  ${COMMON_FLAGS}
  $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${RELEASE_FLAGS}>
)

target_link_libraries(utils
  PRIVATE
    ${SYSTEM_LIBS}
)

target_link_options(utils PRIVATE
  $<$<CONFIG:Release>:${RELEASE_LINKER_FLAGS}>
)

if (EMSCRIPTEN)
  target_compile_options(utils PRIVATE
    -s USE_PTHREADS
  )

  target_link_options(utils PRIVATE
    -s ASYNCIFY
    -s PROXY_TO_PTHREAD
  )
endif()