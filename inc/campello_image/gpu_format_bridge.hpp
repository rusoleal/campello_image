#pragma once

// This header is optional. It provides bidirectional conversion between
// campello_image::TextureFormat and campello_gpu::PixelFormat.
//
// It is guarded by __has_include so that campello_image remains fully
// standalone when campello_gpu is not present in the build.
//
// Usage:
//   #include <campello_image/gpu_format_bridge.hpp>
//   auto pf = campello_image::textureFormatToPixelFormat(texFmt);
//   auto tf = campello_image::pixelFormatToTextureFormat(pixFmt);

#if __has_include(<campello_gpu/constants/pixel_format.hpp>)

#include <campello_gpu/constants/pixel_format.hpp>
#include <campello_image/constants/texture_format.hpp>

namespace systems::leal::campello_image
{

    /**
     * @brief Converts a campello_image TextureFormat to the matching
     * campello_gpu PixelFormat.
     *
     * Values are aligned 1:1 for overlapping formats, so this is typically
     * a zero-cost static_cast. For ASTC the linear campello_image enum maps
     * to the sRGB campello_gpu enum because the block payload is identical.
     *
     * @return The matching PixelFormat, or PixelFormat::invalid if no
     *         correspondence exists.
     */
    inline campello_gpu::PixelFormat textureFormatToPixelFormat(TextureFormat fmt)
    {
        using campello_gpu::PixelFormat;

        switch (fmt)
        {
            case TextureFormat::rgba8:           return PixelFormat::rgba8unorm;
            case TextureFormat::rgba16f:         return PixelFormat::rgba16float;
            case TextureFormat::rgba32f:         return PixelFormat::rgba32float;
            case TextureFormat::bc1_rgba_unorm:  return PixelFormat::bc1_rgba_unorm;
            case TextureFormat::bc3_rgba_unorm:  return PixelFormat::bc3_rgba_unorm;
            case TextureFormat::bc4_r_unorm:     return PixelFormat::bc4_r_unorm;
            case TextureFormat::bc5_rg_unorm:    return PixelFormat::bc5_rg_unorm;
            case TextureFormat::bc6h_rgb_ufloat: return PixelFormat::bc6h_rgb_ufloat;
            case TextureFormat::bc7_rgba_unorm:  return PixelFormat::bc7_rgba_unorm;
            case TextureFormat::etc2_rgb8unorm:  return PixelFormat::etc2_rgb8unorm;
            case TextureFormat::etc2_rgb8a1unorm:return PixelFormat::etc2_rgb8a1unorm;
            case TextureFormat::eac_r11unorm:    return PixelFormat::eac_r11unorm;
            case TextureFormat::eac_rg11unorm:   return PixelFormat::eac_rg11unorm;
            case TextureFormat::astc_4x4_unorm:  return PixelFormat::astc_4x4_unorm_srgb;
        }
        return PixelFormat::invalid;
    }

    /**
     * @brief Converts a campello_gpu PixelFormat to the matching
     * campello_image TextureFormat.
     *
     * Only formats that campello_image can produce are mapped. Depth/stencil
     * formats, packed formats, and sRGB-specific variants without a linear
     * counterpart return TextureFormat::rgba8 as a safe fallback.
     */
    inline TextureFormat pixelFormatToTextureFormat(campello_gpu::PixelFormat fmt)
    {
        using campello_gpu::PixelFormat;

        switch (fmt)
        {
            case PixelFormat::rgba8unorm:      return TextureFormat::rgba8;
            case PixelFormat::rgba16float:     return TextureFormat::rgba16f;
            case PixelFormat::rgba32float:     return TextureFormat::rgba32f;
            case PixelFormat::bc1_rgba_unorm:  return TextureFormat::bc1_rgba_unorm;
            case PixelFormat::bc3_rgba_unorm:  return TextureFormat::bc3_rgba_unorm;
            case PixelFormat::bc4_r_unorm:     return TextureFormat::bc4_r_unorm;
            case PixelFormat::bc5_rg_unorm:    return TextureFormat::bc5_rg_unorm;
            case PixelFormat::bc6h_rgb_ufloat: return TextureFormat::bc6h_rgb_ufloat;
            case PixelFormat::bc7_rgba_unorm:  return TextureFormat::bc7_rgba_unorm;
            case PixelFormat::etc2_rgb8unorm:  return TextureFormat::etc2_rgb8unorm;
            case PixelFormat::etc2_rgb8a1unorm:return TextureFormat::etc2_rgb8a1unorm;
            case PixelFormat::eac_r11unorm:    return TextureFormat::eac_r11unorm;
            case PixelFormat::eac_rg11unorm:   return TextureFormat::eac_rg11unorm;
            case PixelFormat::astc_4x4_unorm_srgb: return TextureFormat::astc_4x4_unorm;
            default:
                return TextureFormat::rgba8;
        }
    }

    /**
     * @brief Returns true if the PixelFormat has a corresponding TextureFormat.
     */
    inline bool isPixelFormatTranscodable(campello_gpu::PixelFormat fmt)
    {
        using campello_gpu::PixelFormat;
        switch (fmt)
        {
            case PixelFormat::rgba8unorm:
            case PixelFormat::rgba16float:
            case PixelFormat::rgba32float:
            case PixelFormat::bc1_rgba_unorm:
            case PixelFormat::bc3_rgba_unorm:
            case PixelFormat::bc4_r_unorm:
            case PixelFormat::bc5_rg_unorm:
            case PixelFormat::bc6h_rgb_ufloat:
            case PixelFormat::bc7_rgba_unorm:
            case PixelFormat::etc2_rgb8unorm:
            case PixelFormat::etc2_rgb8a1unorm:
            case PixelFormat::eac_r11unorm:
            case PixelFormat::eac_rg11unorm:
            case PixelFormat::astc_4x4_unorm_srgb:
                return true;
            default:
                return false;
        }
    }

} // namespace systems::leal::campello_image

#endif // __has_include(<campello_gpu/constants/pixel_format.hpp>)
