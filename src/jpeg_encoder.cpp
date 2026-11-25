// encodeJpeg() (texture_encoder.hpp): stb_image_write's JPEG writer, in every
// build. Its symbols stay private to this file (STB_IMAGE_WRITE_STATIC): other
// libraries linked into the same program may carry their own copy.

#include <campello_image/texture_encoder.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#include "stb_image_write.h"
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace systems::leal::campello_image
{

std::vector<uint8_t> encodeJpeg(const uint8_t* rgba, uint32_t width, uint32_t height, int quality)
{
    std::vector<uint8_t> out;
    if (!rgba || width == 0 || height == 0 || width > 65535u || height > 65535u) return out;  // JPEG's limits
    const auto append = [](void* context, void* data, int size) {
        auto* bytes = static_cast<uint8_t*>(data);
        static_cast<std::vector<uint8_t>*>(context)->insert(static_cast<std::vector<uint8_t>*>(context)->end(),
                                                            bytes, bytes + size);
    };
    out.reserve(size_t(width) * height / 4);
    if (!stbi_write_jpg_to_func(append, &out, int(width), int(height), 4, rgba, std::clamp(quality, 1, 100))) {
        out.clear();
    }
    return out;
}

}
