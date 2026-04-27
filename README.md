# campello_image

A cross-platform C++20 image loading library. Decodes JPEG, PNG, BMP, TGA, GIF, WebP, HDR, and OpenEXR from files or memory buffers. LDR formats decode to RGBA8; HDR formats decode to RGBA32F.

Designed as a companion to [campello_gpu](https://github.com/rusoleal/campello_gpu) — no dependency on it.

## 🚀 Part of the Campello Engine

This project is a module within the **Campello** ecosystem.

👉 Main repository: https://github.com/rusoleal/campello

Campello is a modular, composable game engine built as a collection of independent libraries.
Each module is designed to work standalone, but integrates seamlessly into the engine runtime.

## Supported formats

### Uncompressed images

| Format | Decoder | Output format |
|--------|---------|---------------|
| JPEG   | stb_image (vendored) | RGBA8 |
| PNG    | stb_image (vendored) | RGBA8 |
| BMP    | stb_image (vendored) | RGBA8 |
| TGA    | stb_image (vendored) | RGBA8 |
| GIF    | stb_image (vendored, first frame only) | RGBA8 |
| WebP   | libwebp v1.6.0 | RGBA8 |
| HDR    | stb_image (vendored) | RGBA32F |
| OpenEXR| tinyexr v1.0.0 (vendored) | RGBA32F |

### GPU-compressed textures

| Format | Transcoder | Output formats |
|--------|-----------|----------------|
| Basis Universal (`.basis`) | basis_universal v1.16.4 | BC1–BC7, ETC2, EAC, ASTC_4x4, RGBA8 |
| Khronos KTX2 (`.ktx2`) | basis_universal v1.16.4 | BC1–BC7, ETC2, EAC, ASTC_4x4, RGBA8 |

## Platforms

| Platform | Status |
|----------|--------|
| macOS    | Supported |
| iOS      | Supported |
| Android  | Supported |
| Windows  | Supported |
| Linux    | Supported |

## Requirements

- CMake 3.22.1+
- C++20 compiler

## Build

```bash
cmake -B build
make -C build
```

CMake fetches libwebp automatically via `FetchContent`. stb_image and tinyexr are vendored in `src/`.

## Tests

```bash
./test.sh
```

Or manually:

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage

All types are in the `systems::leal::campello_image` namespace.

```cpp
#include <campello_image/image.hpp>

using namespace systems::leal::campello_image;

// Load from file
auto img = Image::fromFile("texture.png");
if (img) {
    uint32_t      w = img->getWidth();
    uint32_t      h = img->getHeight();
    ImageFormat   fmt = img->getFormat();
    const void*   data = img->getData();
    size_t        size = img->getDataSize();

    if (fmt == ImageFormat::rgba8) {
        const uint8_t* pixels = static_cast<const uint8_t*>(data);
        // 4 bytes per pixel
    } else if (fmt == ImageFormat::rgba32f) {
        const float* pixels = static_cast<const float*>(data);
        // 16 bytes per pixel
    }
}

// Load from memory (e.g. from an asset bundle or network buffer)
auto img2 = Image::fromMemory(encodedBytes, encodedSize);
```

### GPU-compressed texture loading (Basis / KTX2)

```cpp
#include <campello_image/texture_data.hpp>

using namespace systems::leal::campello_image;

// Load a Basis file and transcode to BC7 (desktop)
auto tex = TextureData::fromFile("texture.basis", TextureFormat::bc7_rgba_unorm);
if (tex) {
    uint32_t w = tex->getWidth();
    uint32_t h = tex->getHeight();
    uint32_t mips = tex->getMipLevelCount();

    for (uint32_t mip = 0; mip < mips; ++mip) {
        const void* data = tex->getData(mip);
        size_t      size = tex->getDataSize(mip);
        // Upload directly to GPU — no further conversion needed
    }
}

// Load a KTX2 file and transcode to ASTC 4×4 (mobile / Apple Silicon)
auto tex2 = TextureData::fromFile("texture.ktx2", TextureFormat::astc_4x4_unorm);

// Transcode Basis to ETC2 (Android / older mobile)
auto tex3 = TextureData::fromFile("texture.basis", TextureFormat::etc2_rgb8unorm);
```

Both factories return `nullptr` on failure — unknown format, corrupt data, or missing file.

### Integration with campello_gpu

```cpp
#include <campello_image/image.hpp>
#include <campello_gpu/device.hpp>

using namespace systems::leal::campello_image;
using namespace systems::leal::campello_gpu;

auto img = Image::fromFile("sprite.png");
if (img) {
    auto texture = device->createTexture(
        TextureType::_2D, PixelFormat::rgba8unorm,
        img->getWidth(), img->getHeight(),
        1, 1, 1,
        TextureUsage::textureBinding | TextureUsage::copyDst);

    texture->upload(img->getData(), img->getDataSize());
}
```

### GPU format bridge

When `campello_gpu` headers are visible in your include path, `inc/campello_image/gpu_format_bridge.hpp` provides bidirectional conversion between `TextureFormat` and `campello_gpu::PixelFormat`. The `TextureFormat` enum uses explicit underlying values that match `PixelFormat` for overlapping formats, so the conversion is typically zero-cost:

```cpp
#include <campello_image/texture_data.hpp>
#include <campello_image/gpu_format_bridge.hpp>

using namespace systems::leal::campello_image;
using namespace systems::leal::campello_gpu;

auto tex = TextureData::fromFile("albedo.ktx2", TextureFormat::bc7_rgba_unorm);
if (tex) {
    PixelFormat gpuFmt = textureFormatToPixelFormat(tex->getFormat());
    // gpuFmt == PixelFormat::bc7_rgba_unorm — static_cast under the hood

    // Or the reverse:
    TextureFormat texFmt = pixelFormatToTextureFormat(PixelFormat::astc_4x4_unorm);
}
```

When `campello_gpu` headers are **not** present, the bridge header is a no-op and `campello_image` compiles standalone.

### Using as a CMake dependency (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(
    campello_image
    GIT_REPOSITORY https://github.com/rusoleal/campello_image.git
    GIT_TAG        v0.5.0
)
FetchContent_MakeAvailable(campello_image)

target_link_libraries(your_target PRIVATE campello_image)
```

## Thread safety

`Image` objects are immutable after construction. All getters (`getWidth`, `getHeight`, `getFormat`, `getData`, `getDataSize`) may be called concurrently from multiple threads without synchronisation. Creating or destroying `Image` objects concurrently is safe as long as distinct objects are used.
