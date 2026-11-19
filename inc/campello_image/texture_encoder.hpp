#pragma once

#include <campello_image/constants/texture_format.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace systems::leal::campello_image
{
    class TextureData;

    /**
     * @brief How encodeKtx2() compresses an image.
     */
    struct EncodeOptions
    {
        /// Colour data in sRGB (base colour, emissive): perceptual error
        /// metrics, sRGB mip filtering, an sRGB transfer function in the
        /// file. false: linear data (normal maps, roughness, masks).
        bool srgb = true;
        /// Generate the mip chain (down to 1x1).
        bool mipmaps = true;
        /// Mips filtered with wrapping (textures tiled across a surface);
        /// false: clamped at the edges.
        bool wrap = true;
        /// UASTC effort: 0 (fastest) to 4 (slowest, best quality).
        uint32_t effort = 1;
        /// Use several threads.
        bool threaded = true;
    };

    /**
     * @brief True when this build can encode (the Basis Universal encoder
     * was compiled in: CAMPELLO_IMAGE_ENCODER, on by default).
     */
    bool canEncode();

    /**
     * @brief Encodes RGBA8 pixels (rows top to bottom, 4 bytes per texel)
     * as a KTX2 file: UASTC LDR 4x4, Zstd supercompressed, with its mip
     * chain. A KTX2 transcodes on load to BC7, ASTC 4x4, ETC2 or RGBA8, so
     * one file serves every GPU (TextureData::fromMemory()).
     *
     * @return The file's bytes, or empty on failure (or when canEncode() is
     * false).
     */
    std::vector<uint8_t> encodeKtx2(const uint8_t* rgba, uint32_t width, uint32_t height,
                                    const EncodeOptions& options = {});

    /**
     * @brief Compresses RGBA8 pixels for the GPU in one step: encodeKtx2(),
     * then transcoded to `targetFormat` (BC7, ASTC 4x4, ETC2..., or rgba8),
     * with its mip chain.
     *
     * @return The texture data, or nullptr on failure.
     */
    std::shared_ptr<TextureData> compressRGBA8(const uint8_t* rgba, uint32_t width, uint32_t height,
                                               TextureFormat targetFormat, const EncodeOptions& options = {});
}
