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

#### `Image` (uncompressed)

`Image::fromMemory` tries decoders in order:

1. **stb_image** — handles JPEG, PNG, BMP, TGA, GIF (first frame only), HDR. Vendored at `src/stb_image.h`.
2. **tinyexr** — handles OpenEXR. Vendored at `src/tinyexr.h`.
3. **libwebp** (`webpdecoder` target, decoder-only) — fallback for WebP. Fetched via CMake `FetchContent` at tag `v1.6.0`.

`Image::fromFile` reads the file into memory and delegates to `fromMemory`.

Output format depends on source: LDR → RGBA8, HDR → RGBA32F.

#### `TextureData` (compressed + mipmapped)

`TextureData::fromMemory` detects format by magic bytes and transcodes:

1. **KTX2** — Khronos KTX2 container with Basis Universal payload (ETC1S or UASTC). Parsed and transcoded via basis_universal's `ktx2_transcoder`.
2. **Basis Universal** — `.basis` files. Transcoded via basis_universal's `basisu_transcoder`.
3. **Uncompressed images** — PNG, JPEG, WebP, HDR, EXR. Delegates to `Image::fromMemory`, then wraps the RGBA8 pixels. Only `TextureFormat::rgba8` is supported for this fallback.

The caller selects the target GPU format (BC7, ETC2, ASTC, RGBA8, etc.) and the library transcodes all mip levels automatically.

### Key Types

| Type | Location | Role |
|------|----------|------|
| `Image` | `inc/campello_image/image.hpp` | Public API — decoded CPU-side image (RGBA8 / RGBA32F) |
| `TextureData` | `inc/campello_image/texture_data.hpp` | Public API — GPU-ready block-compressed or uncompressed texture data with mips |
| `ImageFormat` | `inc/campello_image/constants/image_format.hpp` | Enum (`rgba8`, `rgba16f`, `rgba32f`) |
| `TextureFormat` | `inc/campello_image/constants/texture_format.hpp` | Enum of GPU formats (BC, ETC2, ASTC, RGBA8, etc.) |
| `ImageData` | `src/image_handle.hpp` | Internal handle for `Image` |
| `TextureDataHandle` | `src/texture_data_handle.hpp` | Internal handle for `TextureData` — mips, format, dimensions |
| `PixelSource` | `src/image_handle.hpp` | Tracks allocator (`stb` vs `webp` vs `tinyexr`) for correct free in destructor |

### Memory Management

`ImageData::source` determines how the pixel buffer is freed in `Image::~Image`:
- `PixelSource::stb` → `stbi_image_free(pixels)`
- `PixelSource::webp` → `WebPFree(pixels)`

`Image` is non-copyable (`= delete`). Always used via `std::shared_ptr<Image>`.

### Dependencies

| Dependency | How managed | Notes |
|------------|-------------|-------|
| stb_image | Vendored at `src/stb_image.h` | Header-only; `STB_IMAGE_IMPLEMENTATION` defined in `image.cpp` |
| tinyexr | Vendored at `src/tinyexr.h` | Header-only; `TINYEXR_IMPLEMENTATION` defined in `image.cpp` |
| libwebp | CMake `FetchContent`, tag `v1.6.0` | Links `webpdecoder` only (no encoder) |
| basis_universal | CMake `FetchContent`, tag `1.16.4` | Transcoder only (`basisu_transcoder.cpp` + `zstd.c`); encoder tool excluded from build |
| googletest | CMake `FetchContent`, tag `v1.17.0` | Test builds only |

### Tests

- `tests/universal/test_image.cpp` — API contract tests (null inputs, invalid data, embedded byte arrays) and format tests loading real image files from `tests/images/`.
- `tests/images/` — vendored 32×32 test images in all supported formats (JPEG, PNG, BMP, TGA, GIF, WebP), generated with Pillow.
- `CAMPELLO_IMAGE_TEST_ASSETS_DIR` — CMake compile definition injected into the test binary pointing to `tests/images/`.
