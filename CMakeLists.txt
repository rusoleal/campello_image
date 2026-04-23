cmake_minimum_required(VERSION 3.22.1)

project(campello_image VERSION 0.4.0)

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
