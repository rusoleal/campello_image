#pragma once

#include <campello_image/constants/image_format.hpp>
#include <cstdint>

namespace systems::leal::campello_image
{
    enum class PixelSource
    {
        stb,     ///< Decoded by stb_image; free with stbi_image_free().
        webp,    ///< Decoded by libwebp;   free with WebPFree().
        tinyexr, ///< Decoded by tinyexr;   free with std::free().
    };

    struct ImageData
    {
        void*       pixels; ///< Pixel buffer (format depends on @ref format).
        uint32_t    width;
        uint32_t    height;
        ImageFormat format;
        PixelSource source;
    };

} // namespace systems::leal::campello_image
