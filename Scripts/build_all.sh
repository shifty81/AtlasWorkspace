#!/usr/bin/env bash
# AtlasWorkspace — Build all targets
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="$ROOT_DIR/Builds/$BUILD_TYPE"

echo "╔══════════════════════════════════════════╗"
echo "║       AtlasWorkspace Build System             ║"
echo "╠══════════════════════════════════════════╣"
echo "║  Build Type:  $BUILD_TYPE"
echo "║  Build Dir:   $BUILD_DIR"
echo "╚══════════════════════════════════════════╝"

# Configure
cmake -B "$BUILD_DIR" -S "$ROOT_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -D \
    -D \
    -D \
    -DATLAS_BUILD_TESTS=ON

# Build
cmake --build "$BUILD_DIR" --parallel "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "Build complete. Executables in: $BUILD_DIR/bin/"
echo ""

# Optionally run tests
if [[ "${2:-}" == "--test" ]]; then
    echo "Running tests..."
    ctest --test-dir "$BUILD_DIR" --output-on-failure
fi
