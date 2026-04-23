# campello_image

A cross-platform C++20 image loading library. Decodes JPEG, PNG, BMP, TGA, GIF, WebP, HDR, and OpenEXR from files or memory buffers. LDR formats decode to RGBA8; HDR formats decode to RGBA32F.

Designed as a companion to [campello_gpu](https://github.com/rusoleal/campello_gpu) — no dependency on it.

## 🚀 Part of the Campello Engine

This project is a module within the **Campello** ecosystem.

👉 Main repository: https://github.com/rusoleal/campello

Campello is a modular, composable game engine built as a collection of independent libraries.
Each module is designed to work standalone, but integrates seamlessly into the engine runtime.

## Supported formats

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

### Using as a CMake dependency (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(
    campello_image
    GIT_REPOSITORY https://github.com/rusoleal/campello_image.git
    GIT_TAG        v0.4.0
)
FetchContent_MakeAvailable(campello_image)

target_link_libraries(your_target PRIVATE campello_image)
```

## Thread safety

`Image` objects are immutable after construction. All getters (`getWidth`, `getHeight`, `getFormat`, `getData`, `getDataSize`) may be called concurrently from multiple threads without synchronisation. Creating or destroying `Image` objects concurrently is safe as long as distinct objects are used.
