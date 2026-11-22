#pragma once

// Compatibility shim over the CMake-generated export header.
//
// generate_export_header() emits `project_export.h` in the build tree
// (see CMakeLists.txt). It defines PROJECT_EXPORT with the correct
// per-platform, per-linkage expansion:
//
//   static build         -> empty
//   building the shared  -> __declspec(dllexport) / __attribute__((visibility("default")))
//   consuming the shared -> __declspec(dllimport) / empty
//
// We alias it to `EXPORT` so existing user code that writes `EXPORT`
// continues to compile unchanged. The macro name and header filename
// are pinned via BASE_NAME=PROJECT in CMakeLists.txt, so this file
// survives project renames without edits.

#include "project_export.h"

#define EXPORT PROJECT_EXPORT