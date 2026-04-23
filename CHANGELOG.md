# Changelog

All notable changes to campello_image are documented here.

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
