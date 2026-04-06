# campello_image

A cross-platform C++20 image loading library. Decodes JPEG, PNG, BMP, TGA, GIF, and WebP from files or memory buffers, always returning raw RGBA8 pixel data ready for direct GPU texture upload.

Designed as a companion to [campello_gpu](https://github.com/rusoleal/campello_gpu) — no dependency on it.

## Supported formats

| Format | Decoder |
|--------|---------|
| JPEG   | stb_image (vendored) |
| PNG    | stb_image (vendored) |
| BMP    | stb_image (vendored) |
| TGA    | stb_image (vendored) |
| GIF    | stb_image (vendored, first frame only) |
| WebP   | libwebp v1.6.0 |

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

CMake fetches libwebp automatically via `FetchContent`. stb_image is vendored in `src/stb_image.h`.

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
    uint32_t       w    = img->getWidth();
    uint32_t       h    = img->getHeight();
    const uint8_t* data = img->getData();     // RGBA8, w * h * 4 bytes
    size_t         size = img->getDataSize(); // w * h * 4
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
    GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(campello_image)

target_link_libraries(your_target PRIVATE campello_image)
```

## Thread safety

`Image` objects are immutable after construction. All getters (`getWidth`, `getHeight`, `getFormat`, `getData`, `getDataSize`) may be called concurrently from multiple threads without synchronisation. Creating or destroying `Image` objects concurrently is safe as long as distinct objects are used.
