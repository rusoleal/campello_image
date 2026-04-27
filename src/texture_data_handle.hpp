#pragma once

#include <campello_image/constants/texture_format.hpp>
#include <cstdint>
#include <vector>

namespace systems::leal::campello_image
{

    struct MipLevel
    {
        std::vector<uint8_t> data;
    };

    struct TextureDataHandle
    {
        uint32_t      width        = 0;
        uint32_t      height       = 0;
        uint32_t      depth        = 1;
        uint32_t      mipLevels    = 0;
        uint32_t      arrayLayers  = 1;
        TextureFormat format       = TextureFormat::rgba8;
        std::vector<MipLevel> mips;
    };

} // namespace systems::leal::campello_image
