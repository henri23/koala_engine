#!/bin/bash

# Resolve script and root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
BIN_ASSETS_DIR="$ROOT_DIR/bin/assets"
BIN_SHADERS_DIR="$BIN_ASSETS_DIR/shaders"

# Create output directories
mkdir -p "$BIN_SHADERS_DIR"

echo "Compiling shaders..."

# Vertex shader
$VULKAN_SDK/bin/glslc -fshader-stage=vert "$SCRIPT_DIR/assets/shaders/Builtin.ObjectShader.vert.glsl" -o "$BIN_SHADERS_DIR/Builtin.ObjectShader.vert.spv"
if [ $? -ne 0 ]; then
    echo "Error: vertex shader compilation failed"
    exit 1
fi

# Fragment shader
$VULKAN_SDK/bin/glslc -fshader-stage=frag "$SCRIPT_DIR/assets/shaders/Builtin.ObjectShader.frag.glsl" -o "$BIN_SHADERS_DIR/Builtin.ObjectShader.frag.spv"
if [ $? -ne 0 ]; then
    echo "Error: fragment shader compilation failed"
    exit 1
fi

echo "Copying assets..."
cp -R "$SCRIPT_DIR/assets" "$ROOT_DIR/bin"

echo "Done."
