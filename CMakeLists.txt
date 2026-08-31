cmake_minimum_required(VERSION 3.22.1)

project(campello_image VERSION 0.5.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable unity build to reduce compilation time for dependencies (e.g., libwebp)
set(CMAKE_UNITY_BUILD ON)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

configure_file(src/campello_image_config.h.in campello_image_config.h)

# ---------------------------------------------------------------------------
# Third-party dependencies
# ---------------------------------------------------------------------------

include(FetchContent)

# basis_universal — Basis Universal + KTX2 transcoding
#
# v2_50 (was 1.16.4): the 1.x line has no HDR transcode targets at all (no
# BC6H, no ASTC HDR) -- real HDR support (UASTC HDR -> BC6H/ASTC HDR/RGB9E5)
# was added starting v2_0 (Jan 2024) and refined through v2_50 (Aug 2024).
# Also: v1.6's own release notes describe a transcoder-initialization bug
# specific to "apps that only used the transcoder module" (exactly this
# library's usage pattern), fixed in v2.0 -- a real correctness fix even
# for existing LDR-only usage, independent of wanting HDR at all.
FetchContent_Declare(
    basis_universal
    GIT_REPOSITORY https://github.com/BinomialLLC/basis_universal.git
    GIT_TAG        v2_50
)
# basis_universal declares cmake_minimum_required(VERSION 2.8) (still true as
# of v2_50), which CMake 4.0+ no longer supports. Floor old requirements to 3.5.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(basis_universal)

# Exclude everything from basis_universal's own CMakeLists.txt except the
# transcoder sources we compile directly into campello_image ourselves (see
# macos.cmake/linux.cmake/etc.) -- we only need transcoder/basisu_transcoder.cpp
# + zstd/zstd.c, not any of these targets. basisu_encoder in particular fails
# to build under this project's CMAKE_UNITY_BUILD ON (propagates into the
# fetched subdirectory and produces basist::color_rgba/basisu::color_rgba
# ambiguities across unity translation units) -- excluding it from ALL avoids
# building code we never link against anyway.
foreach(_campello_image_unused_basisu_target IN ITEMS basisu basisu_encoder example example_capi example_transcoding)
    if(TARGET ${_campello_image_unused_basisu_target})
        set_target_properties(${_campello_image_unused_basisu_target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endforeach()

# libwebp — WebP decoding
set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_CWEBP      OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_DWEBP      OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_GIF2WEBP   OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_IMG2WEBP   OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_VWEBP      OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPINFO   OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPMUX    OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_EXTRAS     OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    libwebp
    GIT_REPOSITORY https://github.com/webmproject/libwebp.git
    GIT_TAG        v1.6.0
)
FetchContent_MakeAvailable(libwebp)

# Disable unity build for libwebp targets that have internal naming conflicts
set(LIBWEBP_TARGETS
    sharpyuv
    webpdecode
    webpencode
    webpdsp
    webpdspdecode
    webputilsdecode
)
foreach(target IN LISTS LIBWEBP_TARGETS)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES UNITY_BUILD OFF)
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Platform dispatch
# ---------------------------------------------------------------------------

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    include(android.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    include(macos.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    include(ios.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    include(windows.cmake)
else()
    include(linux.cmake)
endif()

# ---------------------------------------------------------------------------
# Tests
#
# BUILD_TESTS=ON — universal tests (image loading, format checks).
#                  No GPU required.
# ---------------------------------------------------------------------------
option(BUILD_TESTS "Build unit tests (universal, no GPU)" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
