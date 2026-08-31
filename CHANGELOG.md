# Changelog

All notable changes to campello_image are documented here.

## [Unreleased]

## [0.5.1] - 2026-08-31

### Changed

- **basis_universal upgraded from `1.16.4` to `v2_50`** — the 1.x line has no HDR transcode
  targets at all (no BC6H, no ASTC HDR); real HDR support (UASTC HDR → BC6H/ASTC HDR/RGB9E5) was
  added starting `v2_0` and refined through `v2_50`. `v1.6`'s own release notes also describe a
  transcoder-initialization bug specific to "apps that only used the transcoder module" — exactly
  this library's usage pattern — fixed in `v2_0`, a real correctness fix independent of HDR.
  Compiled source paths (`transcoder/basisu_transcoder.cpp`, `zstd/zstd.c`) are unchanged; full
  test suite passes clean against the new version with no code changes required for the upgrade
  itself.

### Fixed

- **Build**: `basisu_encoder` and the `example`/`example_capi`/`example_transcoding` targets from
  basis_universal's own `CMakeLists.txt` are now excluded from `ALL` (`CMakeLists.txt`). Under
  `v2_50`, `basisu_encoder` doesn't compile cleanly with this project's `CMAKE_UNITY_BUILD ON` once
  it propagates into the fetched subdirectory — namespace ambiguities between `basist::color_rgba`
  and `basisu::color_rgba` across unity translation units. None of these targets were ever linked
  into campello_image (transcoder-only by design), so excluding them is free.

### Documented (no behavior change)

- **`TextureFormat::etc2_rgb8a1unorm` has no valid Basis transcode target and never will with this
  library.** Investigated adding a mapping to `basist::cTFETC2_RGBA` — that format is a different,
  16-byte-per-block full-8-bit-alpha layout, not the 8-byte punch-through-alpha layout
  `etc2_rgb8a1unorm` actually is (per its own block-size table in `constants/texture_format.cpp`).
  No punch-through-alpha transcode target exists anywhere in basis_universal's transcoder. Stays
  unsupported; `src/texture_data.cpp` now documents why in place, so this isn't re-attempted
  without re-discovering the same block-size mismatch.
- **`TextureFormat::bc6h_rgb_ufloat` is not safely mappable to `basist::cTFBC6H` without
  source-format detection this library doesn't have.** `cTFBC6H` exists in `v2_50` and its numeric
  value does align with `campello_gpu::PixelFormat::bc6h_rgb_ufloat` (151), but it's only handled by
  the UASTC-HDR low-level transcoder — the ETC1S (SDR/`.basis`) transcoder's `transcode_image()` has
  no case for it and hits `assert(0)`, a hard crash rather than a graceful failure. Confirmed
  directly: mapping it turned the existing `TextureDataBasis.UnsupportedFormatReturnsNullptr` test
  (which uses `bc6h_rgb_ufloat` against an SDR asset as its known-safe-to-fail case) into a crash.
  `fromFile()`/`fromMemory()` have no HDR-vs-LDR source detection to gate on before dispatching to
  this target, and there's no local UASTC-HDR test asset to validate the intended happy path
  either — real follow-up work, not a one-line mapping. Stays unsupported, documented in place.

### Tests

- No new tests added this release — existing 33-test universal suite re-validated clean against
  `v2_50` (all three investigations above were caught/confirmed by tests already in the suite).

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

### Build
- **CMake 4.0 compatibility** — set `CMAKE_POLICY_VERSION_MINIMUM=3.5` before `FetchContent_MakeAvailable(basis_universal)` to work around `basis_universal` 1.16.4's `cmake_minimum_required(VERSION 2.8)`

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
