#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include <webp/decode.h>

#include <campello_image/image.hpp>
#include "image_handle.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

namespace systems::leal::campello_image
{

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static size_t bytesPerPixel(ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::rgba8:   return 4;
        case ImageFormat::rgba16f: return 8;
        case ImageFormat::rgba32f: return 16;
    }
    return 4; // unreachable
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

Image::Image(void* pd) : native(pd) {}

Image::~Image()
{
    if (!native)
        return;

    auto* data = static_cast<ImageData*>(native);

    switch (data->source)
    {
        case PixelSource::stb:
            stbi_image_free(data->pixels);
            break;
        case PixelSource::webp:
            WebPFree(data->pixels);
            break;
        case PixelSource::tinyexr:
            std::free(data->pixels);
            break;
    }

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

// Magic-number helpers for format sniffing.
static bool isHdr(const uint8_t* data, size_t size)
{
    // Radiance HDR starts with "#?RADIANCE" or "#?RGBE"
    return size >= 10 &&
           (std::memcmp(data, "#?RADIANCE", 10) == 0 ||
            std::memcmp(data, "#?RGBE", 6) == 0);
}

static bool isExr(const uint8_t* data, size_t size)
{
    // OpenEXR magic: 0x76 0x2f 0x31 0x01
    return size >= 4 && data[0] == 0x76 && data[1] == 0x2f &&
           data[2] == 0x31 && data[3] == 0x01;
}

std::shared_ptr<Image> Image::fromMemory(const uint8_t* data, size_t size)
{
    if (!data || size == 0)
        return nullptr;

    // stb_image takes int for buffer size; guard against truncation on
    // pathological inputs (> 2 GB encoded image data).
    if (size > static_cast<size_t>(std::numeric_limits<int>::max()))
        return nullptr;

    int w = 0, h = 0, ch = 0;

    // --- HDR (Radiance) — must go before LDR stb_image because stbi_load
    //     tone-maps HDR files to 8-bit otherwise. ---------------------------
    if (isHdr(data, size))
    {
        float* pixelsf = stbi_loadf_from_memory(
            data, static_cast<int>(size), &w, &h, &ch, 4 /* force RGBA */);

        if (pixelsf)
        {
            auto* handle   = new ImageData{};
            handle->pixels = pixelsf;
            handle->width  = static_cast<uint32_t>(w);
            handle->height = static_cast<uint32_t>(h);
            handle->format = ImageFormat::rgba32f;
            handle->source = PixelSource::stb;
            return std::shared_ptr<Image>(new Image(handle));
        }
        return nullptr;
    }

    // --- OpenEXR — sniffed by magic so we don't waste cycles on stb_image ---
    if (isExr(data, size))
    {
        float* exr_pixels = nullptr;
        const int ret = LoadEXRFromMemory(
            &exr_pixels, &w, &h, data, size, nullptr);

        if (ret == TINYEXR_SUCCESS && exr_pixels)
        {
            auto* handle   = new ImageData{};
            handle->pixels = exr_pixels;
            handle->width  = static_cast<uint32_t>(w);
            handle->height = static_cast<uint32_t>(h);
            handle->format = ImageFormat::rgba32f;
            handle->source = PixelSource::tinyexr;
            return std::shared_ptr<Image>(new Image(handle));
        }
        return nullptr;
    }

    // --- Try stb_image (JPEG, PNG, BMP, TGA, GIF, …) ------------------------
    uint8_t* pixels8 = stbi_load_from_memory(
        data, static_cast<int>(size), &w, &h, &ch, 4 /* force RGBA */);

    if (pixels8)
    {
        auto* handle   = new ImageData{};
        handle->pixels = pixels8;
        handle->width  = static_cast<uint32_t>(w);
        handle->height = static_cast<uint32_t>(h);
        handle->format = ImageFormat::rgba8;
        handle->source = PixelSource::stb;
        return std::shared_ptr<Image>(new Image(handle));
    }

    // --- Fall back to libwebp -----------------------------------------------
    pixels8 = WebPDecodeRGBA(data, size, &w, &h);

    if (pixels8)
    {
        auto* handle   = new ImageData{};
        handle->pixels = pixels8;
        handle->width  = static_cast<uint32_t>(w);
        handle->height = static_cast<uint32_t>(h);
        handle->format = ImageFormat::rgba8;
        handle->source = PixelSource::webp;
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
    return static_cast<ImageData*>(native)->format;
}

const void* Image::getData() const
{
    return static_cast<ImageData*>(native)->pixels;
}

size_t Image::getDataSize() const
{
    auto* d = static_cast<ImageData*>(native);
    return static_cast<size_t>(d->width) * d->height * bytesPerPixel(d->format);
}

} // namespace systems::leal::campello_image
