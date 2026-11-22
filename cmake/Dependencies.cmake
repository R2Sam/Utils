include_guard(GLOBAL)

# =====================================================================
# System dependencies (find_package)
# =====================================================================
#
# Utils is the base of the dependency chain, so it has no fetched
# dependencies. (The template's `utils` FetchContent block must NOT be
# here — it would make Utils fetch itself.)
#
# Example:
#
#   find_package(OpenSSL REQUIRED)
#   target_link_libraries(${PROJECT_NAME}_platform INTERFACE OpenSSL::SSL)
