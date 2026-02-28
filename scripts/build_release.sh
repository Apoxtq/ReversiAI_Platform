#!/bin/bash
# Release Build Script for ReversiAI_Platform v1.0.0
# Usage: ./scripts/build_release.sh

set -e

# Configuration
VERSION="1.0.0"
PROJECT_NAME="ReversiAI_Platform"
BUILD_DIR="build_release"
QT_ROOT="${Qt6_ROOT:-C:/Qt/6.10.1/mingw_64}"

echo "========================================="
echo "  ReversiAI_Platform Release Builder"
echo "  Version: $VERSION"
echo "========================================="

# Step 1: Create build directory
echo "[1/6] Creating build directory..."
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Step 2: Configure CMake
echo "[2/6] Configuring CMake..."
cmake .. ^
    -G "MinGW Makefiles" ^
    -DBUILD_QT_GUI=ON ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DQt6_ROOT="$QT_ROOT"

# Step 3: Build
echo "[3/6] Building project..."
cmake --build . --config Release --parallel

# Step 4: Run tests
echo "[4/6] Running tests..."
ctest -C Release --output-on-failure

# Step 5: Deploy Qt dependencies (Windows)
echo "[5/6] Deploying Qt dependencies..."
if [ "$(uname)" = "MINGW64_NT-10.0" ] || [ "$(uname)" = "MSYS_NT-10.0" ]; then
    windeployqt ReversiAI_Platform.exe
fi

# Step 6: Create release package
echo "[6/6] Creating release package..."
cd ..
RELEASE_NAME="${PROJECT_NAME}_v${VERSION}_windows_x64"
mkdir -p "releases/$RELEASE_NAME"
cp "$BUILD_DIR/ReversiAI_Platform.exe" "releases/$RELEASE_NAME/"
cp -r docs "releases/$RELEASE_NAME/"
cp README.md "releases/$RELEASE_NAME/"
cp LICENSE "releases/$RELEASE_NAME/"

echo "========================================="
echo "  Build Complete!"
echo "  Release package: releases/$RELEASE_NAME"
echo "========================================="
