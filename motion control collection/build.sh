#!/bin/bash
set -e

echo "=========================================="
echo "Motion Control Collection - Setup Script"
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [ -d "/opt/homebrew" ]; then
    export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:/opt/homebrew/opt/sdl2/lib/pkgconfig:/opt/homebrew/opt/sdl2_mixer/lib/pkgconfig:/opt/homebrew/opt/opencv/lib/pkgconfig:$PKG_CONFIG_PATH"
    export CMAKE_PREFIX_PATH="/opt/homebrew:$CMAKE_PREFIX_PATH"
fi

install_dependencies() {
    echo "Installing system dependencies..."
    
    if command -v nix-env &> /dev/null; then
        echo "Using Nix package manager..."
        nix-env -iA nixpkgs.SDL2 nixpkgs.opencv nixpkgs.cmake nixpkgs.pkg-config 2>/dev/null && echo "[OK] Nix packages installed" || echo "[INFO] Nix install skipped"
    elif command -v apt-get &> /dev/null; then
        echo "Using apt package manager..."
        sudo apt-get update -qq
        sudo apt-get install -y cmake pkg-config libsdl2-dev libopencv-dev build-essential
        echo "[OK] APT packages installed"
    elif command -v brew &> /dev/null; then
        echo "Using Homebrew..."
        brew install cmake pkg-config sdl2 opencv
        echo "[OK] Homebrew packages installed"
    elif command -v pacman &> /dev/null; then
        echo "Using pacman..."
        sudo pacman -S --noconfirm cmake pkgconf sdl2 opencv
        echo "[OK] Pacman packages installed"
    else
        echo "[WARNING] No supported package manager found. Please install dependencies manually."
    fi
}

check_command() {
    if command -v "$1" &> /dev/null; then
        echo "[OK] $1 is installed"
        return 0
    else
        echo "[MISSING] $1 is not installed"
        return 1
    fi
}

check_pkg() {
    if pkg-config --exists "$1" 2>/dev/null; then
        echo "[OK] $1 found via pkg-config"
        return 0
    else
        echo "[MISSING] $1 not found"
        return 1
    fi
}

echo ""
echo "Step 1: Installing dependencies..."
echo "------------------------------------------"

NEED_INSTALL=0
check_command cmake || NEED_INSTALL=1
check_pkg sdl2 || NEED_INSTALL=1
check_pkg opencv4 || NEED_INSTALL=1

if [ $NEED_INSTALL -eq 1 ]; then
    install_dependencies
fi

echo ""
echo "Verifying dependencies..."
MISSING_DEPS=0
check_command cmake || MISSING_DEPS=1
check_command pkg-config || MISSING_DEPS=1
check_command c++ || check_command clang++ || check_command g++ || MISSING_DEPS=1
check_pkg sdl2 || MISSING_DEPS=1
check_pkg opencv4 || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo ""
    echo "[WARNING] Some dependencies still missing. Build may fail."
fi

echo ""
echo "Step 2: Downloading OpenPose model files..."
echo "------------------------------------------"

if [ ! -d "models" ]; then
    echo "Creating models directory..."
    mkdir -p models
else
    echo "[OK] models directory exists"
fi

PROTO_FILE="models/openpose_pose_coco.prototxt"
WEIGHTS_FILE="models/pose_iter_440000.caffemodel"

PROTO_URL="https://raw.githubusercontent.com/opencv/opencv_extra/4.x/testdata/dnn/openpose_pose_coco.prototxt"
WEIGHTS_URL="https://huggingface.co/camenduru/openpose/resolve/main/models/pose/coco/pose_iter_440000.caffemodel"

if [ -f "$PROTO_FILE" ]; then
    echo "[OK] $PROTO_FILE already exists"
else
    echo "Downloading $PROTO_FILE..."
    if curl -L -o "$PROTO_FILE" "$PROTO_URL" 2>/dev/null || wget -O "$PROTO_FILE" "$PROTO_URL" 2>/dev/null; then
        echo "[OK] Downloaded $PROTO_FILE"
    else
        echo "[WARNING] Failed to download $PROTO_FILE - pose detection will use motion fallback"
    fi
fi

if [ -f "$WEIGHTS_FILE" ]; then
    echo "[OK] $WEIGHTS_FILE already exists"
else
    echo "Downloading $WEIGHTS_FILE (this may take a while, ~200MB)..."
    if curl -L -o "$WEIGHTS_FILE" "$WEIGHTS_URL" 2>/dev/null || wget -O "$WEIGHTS_FILE" "$WEIGHTS_URL" 2>/dev/null; then
        FILESIZE=$(stat -f%z "$WEIGHTS_FILE" 2>/dev/null || stat -c%s "$WEIGHTS_FILE" 2>/dev/null || echo "0")
        if [ "$FILESIZE" -gt 100000000 ]; then
            echo "[OK] Downloaded $WEIGHTS_FILE"
        else
            echo "[WARNING] Downloaded file seems too small, may be incomplete"
            rm -f "$WEIGHTS_FILE"
        fi
    else
        echo "[WARNING] Failed to download $WEIGHTS_FILE - pose detection will use motion fallback"
    fi
fi

echo ""
echo "Step 3: Building the games..."
echo "------------------------------------------"

if [ -d "make" ] && [ -f "make/CMakeCache.txt" ]; then
    CACHED_DIR=$(grep "CMAKE_HOME_DIRECTORY" make/CMakeCache.txt 2>/dev/null | head -1 | cut -d'=' -f2)
    if [ -n "$CACHED_DIR" ] && [ "$CACHED_DIR" != "$SCRIPT_DIR" ]; then
        echo "Build cache is from a different machine. Cleaning..."
        rm -rf make
    fi
fi

if [ ! -d "make" ]; then
    echo "Creating make directory..."
    mkdir -p make
else
    echo "[OK] make directory exists"
fi
cd make

echo "Running CMake..."
cmake ..

echo "Compiling..."
make -j$(nproc)

cd ..

if [ ! -d "output" ]; then
    echo "Creating output directory..."
    mkdir -p output
else
    echo "[OK] output directory exists"
fi
cp make/motion_control output/

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="
echo ""
echo "Games included:"
echo "  1. Pickleball - Classic paddle game with AI"
echo "  2. Boxing - Punch targets that appear on screen"
echo "  3. Tennis - Swing your arms to return the ball"
echo "  4. Archery - Draw back and aim to hit targets"
echo "  5. Rhythm Dance - Match on-screen poses (Just Dance style)"
echo ""
echo "To run: ./output/motion_control"
echo ""
if [ -f "$WEIGHTS_FILE" ] && [ -f "$PROTO_FILE" ]; then
    echo "OpenPose model files: INSTALLED"
    echo "  Full pose detection is enabled!"
else
    echo "OpenPose model files: NOT INSTALLED"
    echo "  Games will use motion detection fallback"
fi
echo ""
