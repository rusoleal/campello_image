#pragma once

#include <cstdint>

namespace systems::leal::campello_image
{
    enum class PixelSource
    {
        stb,  ///< Decoded by stb_image; free with stbi_image_free().
        webp, ///< Decoded by libwebp;   free with WebPFree().
    };

    struct ImageData
    {
        uint8_t*    pixels; ///< RGBA8 pixel buffer.
        uint32_t    width;
        uint32_t    height;
        PixelSource source;
    };

} // namespace systems::leal::campello_image
