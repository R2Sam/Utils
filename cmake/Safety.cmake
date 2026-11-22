include_guard(GLOBAL)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

add_library(${PROJECT_NAME}_safety INTERFACE)
add_library(${PROJECT_NAME}::safety ALIAS ${PROJECT_NAME}_safety)

# ---------------------------------------------------------------------
# Stack protector — GCC/Clang. Detected rather than assumed, so an
# exotic compiler that rejects the flag degrades gracefully.
# ---------------------------------------------------------------------
check_cxx_compiler_flag("-fstack-protector-strong"
	${PROJECT_NAME}_HAS_STACK_PROTECTOR)
if(${PROJECT_NAME}_HAS_STACK_PROTECTOR)
	target_compile_options(${PROJECT_NAME}_safety INTERFACE
		-fstack-protector-strong)
endif()

# ---------------------------------------------------------------------
# Hardened stdlib assertions.
# _GLIBCXX_ASSERTIONS is a libstdc++ feature; detect libstdc++ (via
# its `__GLIBCXX__` macro) rather than assuming it. libc++ / MSVC STL
# would silently ignore the define, so it's skipped entirely there.
# ---------------------------------------------------------------------
check_cxx_source_compiles("
	#include <cstddef>
	#if defined(__GLIBCXX__)
	int main() { return 0; }
	#else
	#error not libstdc++
	#endif
" ${PROJECT_NAME}_USING_LIBSTDCXX)

if(${PROJECT_NAME}_USING_LIBSTDCXX)
	target_compile_definitions(${PROJECT_NAME}_safety INTERFACE
		_GLIBCXX_ASSERTIONS)
endif()

# ---------------------------------------------------------------------
# Debug-only sanitizers (Linux + GCC/Clang).
# No sanitizers in Release or RelWithDebInfo — the previous Release
# UBSan-trap turned harmless UB into SIGILL in shipped binaries.
# ---------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux"
   AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

	check_cxx_compiler_flag("-fsanitize=address"
		${PROJECT_NAME}_HAS_ASAN)
	check_cxx_compiler_flag("-fsanitize=undefined"
		${PROJECT_NAME}_HAS_UBSAN)

	if(${PROJECT_NAME}_HAS_ASAN)
		target_compile_options(${PROJECT_NAME}_safety INTERFACE
			$<$<CONFIG:Debug>:-fsanitize=address>)
		target_link_options(${PROJECT_NAME}_safety INTERFACE
			$<$<CONFIG:Debug>:-fsanitize=address>)
	endif()

	if(${PROJECT_NAME}_HAS_UBSAN)
		target_compile_options(${PROJECT_NAME}_safety INTERFACE
			$<$<CONFIG:Debug>:-fsanitize=undefined>)
		target_link_options(${PROJECT_NAME}_safety INTERFACE
			$<$<CONFIG:Debug>:-fsanitize=undefined>)
	endif()

endif()