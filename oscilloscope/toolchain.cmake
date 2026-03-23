cmake_minimum_required(VERSION 3.18)
include_guard(GLOBAL)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(SYSROOT "/sysroot")
set(CMAKE_SYSROOT "${SYSROOT}")
set(CMAKE_FIND_ROOT_PATH "${SYSROOT}")

# Cross compilers
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# don't execute try-compile (unneeded for cross compilation)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# make cmake find opengl deterministically
set(OpenGL_GL_PREFERENCE GLVND)
set(OPENGL_opengl_LIBRARY "${SYSROOT}/usr/lib/aarch64-linux-gnu/libOpenGL.so")
set(OPENGL_glx_LIBRARY "${SYSROOT}/usr/lib/aarch64-linux-gnu/libGLX.so")
set(OPENGL_gl_LIBRARY "${SYSROOT}/usr/lib/aarch64-linux-gnu/libGL.so")

# use host qt tools instead of target ones to build binary
set(QT_HOST_PATH "/usr" CACHE STRING "" FORCE)
set(QT_QML_IMPORT_SCANNER "/usr/lib/qt6/libexec/qmlimportscanner" CACHE FILEPATH "" FORCE)
set(Qt6_HOST_INFO_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt6HostInfo" CACHE PATH "" FORCE)
set(Qt6HostInfo_DIR "/usr/lib/x86_64-linux-gnu/cmake/Qt6HostInfo" CACHE PATH "" FORCE)

# prevent Qt from trying to run target-built executables
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "")

# ensure host comes from host, target comes from target
set(CMAKE_FIND_ROOT_PATH "${SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY) 
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(ENV{PKG_CONFIG_SYSROOT_DIR} "${SYSROOT}")
set(ENV{PKG_CONFIG_LIBDIR}
    "${SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig:${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig")

# Help CMake find target Qt6Config.cmake inside the sysroot
list(PREPEND CMAKE_PREFIX_PATH
    "${SYSROOT}/usr/lib/aarch64-linux-gnu/cmake"
    "${SYSROOT}/usr"
)
