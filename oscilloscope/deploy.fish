#!/usr/bin/env fish

# -- config ----------------------------------------
set PROJECT_DIR (pwd)
set BUILD_DIR "build-rpi"
set BIN_NAME "qmloscilloscope"

set PI_HOST "jspark@zero2w.local"
set PI_DEST "~/"

set SYSROOT_HOST "$HOME/sysroots/rpi-bookworm-arm64"

set DOCKER_IMAGE "qt-cross-bookworm:dev"

set TOOLCHAIN_FILE "./toolchain.cmake"
set QT6_DIR "/sysroot/usr/lib/aarch64-linux-gnu/cmake/Qt6"

if not test -d "$SYSROOT_HOST"
    echo "ERROR: SYSROOT_HOST not found: $SYSROOT_HOST"
    exit 1
end

if not test -f "$TOOLCHAIN_FILE"
    echo "ERROR: TOOLCHAIN_FILE not found: $TOOLCHAIN_FILE" 
end

# -- docker build ----------------------------------
echo "==> Building in Docker..." 
docker run --rm -it \
    -v "$PROJECT_DIR:/work" \
    -v "$SYSROOT_HOST:/sysroot" \
    -w /work \
    $DOCKER_IMAGE \
    bash -lc "
        set -e
        rm -rf $BUILD_DIR
        cmake -S . -B $BUILD_DIR -G Ninja \
            -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja \
            -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
            -DQt6_DIR=$QT6_DIR
        cmake --build $BUILD_DIR -j
    "

# -- verify binary ---------------------------------
set BIN_PATH "$BUILD_DIR/$BIN_NAME"
if not test -f "$BIN_PATH"
    echo "ERROR: binary not found at: $BIN_PATH"
    echo "Build dir contents:"
    ls -la "$BUILD_DIR"
    exit 1
end

echo "==> Built: $BIN_PATH"
ls -lh "$BIN_PATH"

# -- deploy ----------------------------------------
echo "==> Deploying to Pi ($PI_HOST)..."
rsync -avz "$BIN_PATH" "$PI_HOST:$PI_DEST"

echo "==> Done, binary located at $PI_DEST/$BIN_NAME"
