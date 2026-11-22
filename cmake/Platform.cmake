include_guard(GLOBAL)

add_library(${PROJECT_NAME}_platform INTERFACE)
add_library(${PROJECT_NAME}::platform ALIAS ${PROJECT_NAME}_platform)

# ---------------------------------------------------------------------
# Threads — portable via FindThreads.
#   Linux   -> -lpthread
#   macOS   -> libSystem (no-op)
#   Windows -> Windows API
# ---------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}_platform INTERFACE Threads::Threads)

# ---------------------------------------------------------------------
# Windows / MinGW: statically link the C/C++ runtime when the final
# artifact is a static library. BUILD_SHARED_LIBS correctly reflects
# that — it is what CMakeLists.txt uses to choose the artifact type.
# MSVC has no `-static` equivalent, so it is excluded.
# ---------------------------------------------------------------------
if(WIN32 AND NOT MSVC AND NOT BUILD_SHARED_LIBS)
	target_link_options(${PROJECT_NAME}_platform INTERFACE -static)
endif()

# ---------------------------------------------------------------------
# Optional: compile for the host CPU in optimised builds only.
# CMAKE_SYSTEM_PROCESSOR is a built-in and lets us pick the correct
# flag per architecture (`-march` on x86, `-mcpu` on ARM), rather than
# guessing by compiler ID.
# ---------------------------------------------------------------------
if(NATIVE AND CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo)$")
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
		target_compile_options(${PROJECT_NAME}_platform INTERFACE -march=native)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
		target_compile_options(${PROJECT_NAME}_platform INTERFACE -mcpu=native)
	endif()
endif()

# ---------------------------------------------------------------------
# Project-specific platform libraries — add here as needed:
#
#   if(WIN32)
#       target_link_libraries(${PROJECT_NAME}_platform INTERFACE winmm)
#   endif()
#   if(APPLE)
#       target_link_libraries(${PROJECT_NAME}_platform INTERFACE
#           "-framework CoreFoundation")
#   endif()
# ---------------------------------------------------------------------