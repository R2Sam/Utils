include_guard(GLOBAL)

# Enables a conservative but useful warning baseline on `target`.
# No warning is ever *disabled* here — the goal is to surface real
# problems, not to hand-wave them away. If a specific warning is
# genuinely inapplicable for a given file, suppress it locally with a
# #pragma rather than globally.
function(project_enable_warnings target)
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
		target_compile_options(${target} PRIVATE
			-Wall
			-Wextra
			-pedantic
		)
	elseif(MSVC)
		target_compile_options(${target} PRIVATE
			/W4
			/permissive-
			/utf-8           # treat sources as UTF-8
			/Zc:__cplusplus  # report the correct __cplusplus value
		)
	endif()
endfunction()