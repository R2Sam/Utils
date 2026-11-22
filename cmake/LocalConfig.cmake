# Per-developer local configuration. Gitignored — do not commit.
#
# Setting UTILS_SOURCE_DIR points the build at a local Utils checkout.
# If this file is absent (or the variable is empty), the build fetches
# Utils from GitHub — this file is optional.
#
# Uses plain `set()` (no CACHE) so it reliably shadows anything a
# preset or a previous configure left behind.

set(UTILS_SOURCE_DIR "/home/sam/Documents/Projects/Utils")