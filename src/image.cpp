#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <webp/decode.h>

#include <campello_image/image.hpp>
#include "image_handle.hpp"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <vector>

namespace systems::leal::campello_image
{

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

Image::Image(void* pd) : native(pd) {}

Image::~Image()
{
    if (!native)
        return;

    auto* data = static_cast<ImageData*>(native);

    if (data->source == PixelSource::stb)
        stbi_image_free(data->pixels);
    else
        WebPFree(data->pixels);

    delete data;
}

// ---------------------------------------------------------------------------
// Factory: fromFile
// ---------------------------------------------------------------------------

std::shared_ptr<Image> Image::fromFile(const char* path)
{
    if (!path)
        return nullptr;

    // Read the entire file into memory, then delegate to fromMemory.
    FILE* f = fopen(path, "rb");
    if (!f)
        return nullptr;

    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0)
    {
        fclose(f);
        return nullptr;
    }

    std::vector<uint8_t> buf(static_cast<size_t>(size));
    const size_t read = fread(buf.data(), 1, buf.size(), f);
    fclose(f);

    if (read != buf.size())
        return nullptr;

    return fromMemory(buf.data(), buf.size());
}

// ---------------------------------------------------------------------------
// Factory: fromMemory
// ---------------------------------------------------------------------------

std::shared_ptr<Image> Image::fromMemory(const uint8_t* data, size_t size)
{
    if (!data || size == 0)
        return nullptr;

    // stb_image takes int for buffer size; guard against truncation on
    // pathological inputs (> 2 GB encoded image data).
    if (size > static_cast<size_t>(std::numeric_limits<int>::max()))
        return nullptr;

    int w = 0, h = 0, ch = 0;

    // --- Try stb_image first (JPEG, PNG, BMP, TGA, GIF, …) ----------------
    uint8_t* pixels = stbi_load_from_memory(
        data, static_cast<int>(size), &w, &h, &ch, 4 /* force RGBA */);

    if (pixels)
    {
        auto* handle      = new ImageData{};
        handle->pixels    = pixels;
        handle->width     = static_cast<uint32_t>(w);
        handle->height    = static_cast<uint32_t>(h);
        handle->source    = PixelSource::stb;
        return std::shared_ptr<Image>(new Image(handle));
    }

    // --- Fall back to libwebp -----------------------------------------------
    pixels = WebPDecodeRGBA(data, size,  &w, &h);

    if (pixels)
    {
        auto* handle      = new ImageData{};
        handle->pixels    = pixels;
        handle->width     = static_cast<uint32_t>(w);
        handle->height    = static_cast<uint32_t>(h);
        handle->source    = PixelSource::webp;
        return std::shared_ptr<Image>(new Image(handle));
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

uint32_t Image::getWidth() const
{
    return static_cast<ImageData*>(native)->width;
}

uint32_t Image::getHeight() const
{
    return static_cast<ImageData*>(native)->height;
}

ImageFormat Image::getFormat() const
{
    return ImageFormat::rgba8;
}

const uint8_t* Image::getData() const
{
    return static_cast<ImageData*>(native)->pixels;
}

size_t Image::getDataSize() const
{
    auto* d = static_cast<ImageData*>(native);
    return static_cast<size_t>(d->width) * d->height * 4;
}

} // namespace systems::leal::campello_image
