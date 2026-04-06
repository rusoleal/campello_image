# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
cmake -B build
make -C build
```

Run tests:

```bash
./test.sh
```

Or manually:

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Platform-specific CMake files are selected based on target: `android.cmake`, `macos.cmake`, `ios.cmake`, `windows.cmake`, `linux.cmake`.

## Architecture Overview

`campello_image` is a cross-platform CPU-side image loading library (C++20). It has no dependency on campello_gpu or any GPU API. All public types live in the `systems::leal::campello_image` namespace.

### Handle-Based Abstraction Pattern

The same pattern as campello_gpu is used. The public `Image` class holds a `void*` pointer to an internal `ImageData` handle struct (defined in `src/image_handle.hpp`). This keeps decoder headers (stb_image, libwebp) out of the public API.

- Public headers: `inc/campello_image/*.hpp`
- Internal handle: `src/image_handle.hpp`
- Implementation: `src/image.cpp`

### Decoding Strategy

`Image::fromMemory` tries decoders in order:

1. **stb_image** — handles JPEG, PNG, BMP, TGA, GIF (first frame only). Vendored at `src/stb_image.h`. Always requests 4 channels (RGBA8).
2. **libwebp** (`webpdecoder` target, decoder-only) — fallback for WebP. Fetched via CMake `FetchContent` at tag `v1.5.0`.

`Image::fromFile` reads the file into memory and delegates to `fromMemory`.

Output is always **RGBA8** — 4 bytes per pixel, regardless of the source file's channel count.

### Key Types

| Type | Location | Role |
|------|----------|------|
| `Image` | `inc/campello_image/image.hpp` | Public API — factory + getters |
| `ImageFormat` | `inc/campello_image/constants/image_format.hpp` | Enum (`rgba8`) |
| `ImageData` | `src/image_handle.hpp` | Internal handle — pixels, width, height, PixelSource |
| `PixelSource` | `src/image_handle.hpp` | Tracks allocator (`stb` vs `webp`) for correct free in destructor |

### Memory Management

`ImageData::source` determines how the pixel buffer is freed in `Image::~Image`:
- `PixelSource::stb` → `stbi_image_free(pixels)`
- `PixelSource::webp` → `WebPFree(pixels)`

`Image` is non-copyable (`= delete`). Always used via `std::shared_ptr<Image>`.

### Dependencies

| Dependency | How managed | Notes |
|------------|-------------|-------|
| stb_image | Vendored at `src/stb_image.h` | Header-only; `STB_IMAGE_IMPLEMENTATION` defined in `image.cpp` |
| libwebp | CMake `FetchContent`, tag `v1.5.0` | Links `webpdecoder` only (no encoder) |
| googletest | CMake `FetchContent`, tag `v1.14.0` | Test builds only |

### Tests

- `tests/universal/test_image.cpp` — API contract tests (null inputs, invalid data, embedded byte arrays) and format tests loading real image files from `tests/images/`.
- `tests/images/` — vendored 32×32 test images in all supported formats (JPEG, PNG, BMP, TGA, GIF, WebP), generated with Pillow.
- `CAMPELLO_IMAGE_TEST_ASSETS_DIR` — CMake compile definition injected into the test binary pointing to `tests/images/`.
