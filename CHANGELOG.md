# Changelog

All notable changes to campello_image are documented here.

## [Unreleased]

## [0.5.0] - 2026-04-27

### Added

- **Basis Universal + KTX2 transcoding** — `TextureData` class for GPU-ready block-compressed texture data
  - `TextureData::fromFile(path, targetFormat)` and `TextureData::fromMemory(data, size, targetFormat)` — auto-detect Basis (.basis), KTX2 (.ktx2), and uncompressed image formats
  - **Supported transcoding targets**: RGBA8, BC1, BC3, BC4, BC5, BC7, ETC2_RGB8, EAC_R11, EAC_RG11, ASTC_4x4
  - **Basis Universal transcoder** — fetched via CMake `FetchContent` (BinomialLLC/basis_universal @ 1.16.4)
  - **KTX2 parser** — built on basis_universal's `ktx2_transcoder`; supports ETC1S and UASTC payloads with Zstd supercompression
  - **Mip level support** — all mip levels from the source file are transcoded and accessible via `getData(mipLevel)` / `getDataSize(mipLevel)`
  - **Block info helpers** — `getBlockWidth()`, `getBlockHeight()`, `getBlockBytes()`, `isCompressed()` for computing GPU upload parameters
- **`TextureFormat` enum** — campello_image's own format enum (no dependency on campello_gpu); values map 1:1 to campello_gpu's `PixelFormat` for zero-cost casting when both libraries are used together
  - Block helpers: `getTextureFormatBlockBytes()`, `getTextureFormatBlockWidth()`, `getTextureFormatBlockHeight()`, `isTextureFormatCompressed()`
- **Uncompressed image fallback** — PNG, JPEG, WebP, HDR, EXR files loaded through `TextureData` decode to RGBA8 via the existing `Image` pipeline
- **GPU format bridge** — `inc/campello_image/gpu_format_bridge.hpp` provides `textureFormatToPixelFormat()` and `pixelFormatToTextureFormat()` conversions
  - Guarded by `__has_include(<campello_gpu/constants/pixel_format.hpp>)` so campello_image stays standalone when campello_gpu is absent
  - `TextureFormat` uses explicit underlying values that match `campello_gpu::PixelFormat` for zero-cost casting: `static_cast<PixelFormat>(static_cast<uint32_t>(TextureFormat::bc7_rgba_unorm))`

### Tests

- 17 new `TextureData` universal tests:
  - Basis → RGBA8, BC1, BC7, ETC2, ASTC_4x4 transcoding
  - KTX2 → RGBA8, BC7, ASTC_4x4 transcoding
  - Uncompressed PNG → RGBA8 fallback
  - Null/invalid input rejection
  - Unsupported format rejection (e.g., BC6H from Basis)
  - Out-of-range mip level safety
- Vendored test assets: `tests/images/test.basis` and `tests/images/test.ktx2` (8×8 RGBA with 4 mip levels, encoded with basisu)

## [0.4.0] - 2026-04-23

### Added
- **HDR (Radiance `.hdr`) support** — decoded via stb_image to RGBA32F; detected by `#?RADIANCE` / `#?RGBE` magic bytes
- **OpenEXR (`.exr`) support** — decoded via tinyexr v1.0.0 (vendored single-header) to RGBA32F
- **`ImageFormat::rgba16f`** and **`ImageFormat::rgba32f`** — pixel format enum variants for floating-point output
- **`PixelSource::tinyexr`** — internal allocator tracking for EXR pixel buffers

### Changed
- **`Image::getData()`** return type changed from `const uint8_t*` to `const void*`. Callers must cast based on `getFormat()`:
  - `ImageFormat::rgba8`   → `static_cast<const uint8_t*>(img->getData())`
  - `ImageFormat::rgba32f` → `static_cast<const float*>(img->getData())`
- **`Image::getDataSize()`** now accounts for the actual pixel format (4, 8, or 16 bytes per pixel)
- **`Image::getFormat()`** is no longer hard-coded to `rgba8`; it returns the format determined by the decoder

## [0.3.1] - 2026-04-06

### Changed
- **googletest** — upgraded from v1.14.0 to v1.17.0

### Build
- **Unity build** — enabled to reduce compilation time; selectively disabled for libwebp targets with internal naming conflicts

## [0.3.0] - 2026-04-06

### Changed
- **libwebp** — upgraded from v1.5.0 to v1.6.0 (fetched from GitHub mirror for better reliability)

## [0.2.0] - 2026-04-06

### Added
- **`THIRD_PARTY_LICENSES`** — attribution file for vendored and fetched dependencies (stb_image MIT, libwebp BSD 3-Clause)

## [0.1.0] - 2026-04-06

### Added
- **`Image` class** — CPU-side decoded image with handle-based abstraction pattern
- **`Image::fromFile(path)`** — decodes an image file to RGBA8; returns `nullptr` on failure
- **`Image::fromMemory(data, size)`** — decodes from an in-memory byte buffer; returns `nullptr` on failure
- **Supported formats** — JPEG, PNG, BMP, TGA, GIF via stb_image; WebP via libwebp
- **Always-RGBA8 output** — pixel data is always 4 channels, 8 bits per channel regardless of source format; safe for direct GPU texture upload
- **`ImageFormat` enum** — `rgba8` constant in `campello_image/constants/image_format.hpp`
- **Getters** — `getWidth()`, `getHeight()`, `getFormat()`, `getData()`, `getDataSize()`
- **CMake build** — platform-dispatched (`android.cmake`, `macos.cmake`, `ios.cmake`, `windows.cmake`, `linux.cmake`); dependencies fetched via `FetchContent` (stb @ master, libwebp @ v1.5.0)
- **Universal tests** — 12 GoogleTest cases covering null inputs, invalid data, PNG decoding, and WebP decoding; no GPU required
- **CI** — GitHub Actions pipeline: universal tests on macOS/Linux/Windows, build check for iOS simulator, build check for Android (arm64-v8a, API 28)
