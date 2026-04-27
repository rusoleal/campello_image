#include <campello_image/constants/texture_format.hpp>

namespace systems::leal::campello_image
{

uint32_t getTextureFormatBlockBytes(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::rgba8:    return 4;
        case TextureFormat::rgba16f:  return 8;
        case TextureFormat::rgba32f:  return 16;

        case TextureFormat::bc1_rgba_unorm:  return 8;
        case TextureFormat::bc3_rgba_unorm:  return 16;
        case TextureFormat::bc4_r_unorm:     return 8;
        case TextureFormat::bc5_rg_unorm:    return 16;
        case TextureFormat::bc6h_rgb_ufloat: return 16;
        case TextureFormat::bc7_rgba_unorm:  return 16;

        case TextureFormat::etc2_rgb8unorm:   return 8;
        case TextureFormat::etc2_rgb8a1unorm: return 8;
        case TextureFormat::eac_r11unorm:     return 8;
        case TextureFormat::eac_rg11unorm:    return 16;

        case TextureFormat::astc_4x4_unorm:   return 16;
    }
    return 4;
}

uint32_t getTextureFormatBlockWidth(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::bc1_rgba_unorm:
        case TextureFormat::bc3_rgba_unorm:
        case TextureFormat::bc4_r_unorm:
        case TextureFormat::bc5_rg_unorm:
        case TextureFormat::bc6h_rgb_ufloat:
        case TextureFormat::bc7_rgba_unorm:
        case TextureFormat::etc2_rgb8unorm:
        case TextureFormat::etc2_rgb8a1unorm:
        case TextureFormat::eac_r11unorm:
        case TextureFormat::eac_rg11unorm:
        case TextureFormat::astc_4x4_unorm:
            return 4;
        default:
            return 1;
    }
}

uint32_t getTextureFormatBlockHeight(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::bc1_rgba_unorm:
        case TextureFormat::bc3_rgba_unorm:
        case TextureFormat::bc4_r_unorm:
        case TextureFormat::bc5_rg_unorm:
        case TextureFormat::bc6h_rgb_ufloat:
        case TextureFormat::bc7_rgba_unorm:
        case TextureFormat::etc2_rgb8unorm:
        case TextureFormat::etc2_rgb8a1unorm:
        case TextureFormat::eac_r11unorm:
        case TextureFormat::eac_rg11unorm:
        case TextureFormat::astc_4x4_unorm:
            return 4;
        default:
            return 1;
    }
}

bool isTextureFormatCompressed(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::bc1_rgba_unorm:
        case TextureFormat::bc3_rgba_unorm:
        case TextureFormat::bc4_r_unorm:
        case TextureFormat::bc5_rg_unorm:
        case TextureFormat::bc6h_rgb_ufloat:
        case TextureFormat::bc7_rgba_unorm:
        case TextureFormat::etc2_rgb8unorm:
        case TextureFormat::etc2_rgb8a1unorm:
        case TextureFormat::eac_r11unorm:
        case TextureFormat::eac_rg11unorm:
        case TextureFormat::astc_4x4_unorm:
            return true;
        default:
            return false;
    }
}

} // namespace systems::leal::campello_image
