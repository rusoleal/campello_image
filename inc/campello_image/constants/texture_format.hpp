#pragma once

#include <cstdint>

namespace systems::leal::campello_image
{

    /**
     * @brief GPU texture format that compressed texture data can be transcoded to.
     *
     * These formats map 1:1 to campello_gpu's PixelFormat enum, allowing
     * zero-cost casting when the two libraries are used together. When used
     * standalone, campello_image has no dependency on campello_gpu.
     *
     * Block-compressed formats (BC, ETC2, ASTC) are fixed 4×4 or ASTC-block
     * compressed. Use TextureData::getBlockWidth/Height/Bytes to compute
     * memory requirements.
     */
    /**
     * @brief GPU texture format that compressed texture data can be transcoded to.
     *
     * Numeric values are explicitly assigned to match campello_gpu's
     * PixelFormat enum where the concepts overlap. This enables zero-cost
     * casting via static_cast when both libraries are used together:
     *
     * @code
     *   auto pf = static_cast<campello_gpu::PixelFormat>(
     *                 static_cast<uint32_t>(TextureFormat::bc7_rgba_unorm));
     *   // pf == PixelFormat::bc7_rgba_unorm
     * @endcode
     *
     * For the reverse direction (PixelFormat → TextureFormat) use the
     * conversion functions in gpu_format_bridge.hpp.
     */
    enum class TextureFormat
    {
        // Uncompressed formats
        rgba8    = 70,   ///< Matches PixelFormat::rgba8unorm.
        rgba16f  = 115,  ///< Matches PixelFormat::rgba16float.
        rgba32f  = 125,  ///< Matches PixelFormat::rgba32float.

        // BC compressed formats (desktop GPUs)
        bc1_rgba_unorm  = 130, ///< Matches PixelFormat::bc1_rgba_unorm.
        bc3_rgba_unorm  = 134, ///< Matches PixelFormat::bc3_rgba_unorm.
        bc4_r_unorm     = 140, ///< Matches PixelFormat::bc4_r_unorm.
        bc5_rg_unorm    = 142, ///< Matches PixelFormat::bc5_rg_unorm.
        bc6h_rgb_ufloat = 151, ///< Matches PixelFormat::bc6h_rgb_ufloat.
        bc7_rgba_unorm  = 152, ///< Matches PixelFormat::bc7_rgba_unorm.

        // ETC2 compressed formats (mobile GPUs)
        eac_r11unorm     = 170, ///< Matches PixelFormat::eac_r11unorm.
        etc2_rgb8unorm   = 180, ///< Matches PixelFormat::etc2_rgb8unorm.
        etc2_rgb8a1unorm = 182, ///< Matches PixelFormat::etc2_rgb8a1unorm.
        eac_rg11unorm    = 174, ///< Matches PixelFormat::eac_rg11unorm.

        // ASTC compressed formats (mobile / Apple Silicon)
        // Note: Basis Universal produces the same block data for linear and
        // sRGB ASTC. The distinction is in the GPU sampler, not the payload.
        // This value aligns with PixelFormat::astc_4x4_unorm_srgb.
        astc_4x4_unorm   = 186, ///< Matches PixelFormat::astc_4x4_unorm_srgb.
    };

    /**
     * @brief Returns the number of bytes in a single compression block.
     *
     * For uncompressed formats returns bytes-per-pixel.
     * For compressed formats returns the fixed block byte size.
     */
    uint32_t getTextureFormatBlockBytes(TextureFormat format);

    /**
     * @brief Returns the block width in texels.
     *
     * For uncompressed formats returns 1.
     * For BC/ETC2 returns 4.
     * For ASTC returns the block width encoded in the format name.
     */
    uint32_t getTextureFormatBlockWidth(TextureFormat format);

    /**
     * @brief Returns the block height in texels.
     *
     * For uncompressed formats returns 1.
     * For BC/ETC2 returns 4.
     * For ASTC returns the block height encoded in the format name.
     */
    uint32_t getTextureFormatBlockHeight(TextureFormat format);

    /**
     * @brief Returns true if the format is block-compressed.
     */
    bool isTextureFormatCompressed(TextureFormat format);

} // namespace systems::leal::campello_image
