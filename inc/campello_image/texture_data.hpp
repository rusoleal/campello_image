#pragma once

#include <campello_image/constants/texture_format.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace systems::leal::campello_image
{

    /**
     * @brief GPU-ready texture data, optionally block-compressed.
     *
     * TextureData holds raw block-compressed or uncompressed pixel data that
     * can be uploaded directly to a GPU texture without further conversion.
     * It supports multiple mip levels and is the output of Basis Universal /
     * KTX2 transcoding.
     *
     * Factory methods detect the source format automatically:
     * - `.basis` files → transcoded via Basis Universal
     * - `.ktx2` files → parsed and transcoded via Basis Universal
     * - `.png`, `.jpg`, `.webp`, etc. → decoded to raw RGBA8
     *
     * The caller selects the target GPU format (BC7, ETC2, ASTC, RGBA8, etc.)
     * and the library transcodes or converts accordingly.
     *
     * **Thread safety:** TextureData objects are immutable after construction.
     * All getters may be called concurrently. Creating or destroying distinct
     * TextureData objects concurrently is safe.
     */
    class TextureData
    {
        void* native;

    public:
        explicit TextureData(void* pd);
        /**
         * @brief Loads texture data from a file path.
         *
         * Detects the file format from the contents (not the extension) and
         * transcodes or decodes to the requested target format. Returns
         * nullptr if the file cannot be read or the format is unsupported.
         *
         * @param path         Path to the file.
         * @param targetFormat Desired GPU texture format.
         * @return A TextureData object, or nullptr on failure.
         */
        static std::shared_ptr<TextureData> fromFile(const char* path,
                                                     TextureFormat targetFormat);

        /**
         * @brief Loads texture data from a memory buffer.
         *
         * Detects the format from the magic bytes and transcodes or decodes
         * to the requested target format. Returns nullptr if the data is
         * invalid or the format is unsupported.
         *
         * @param data         Pointer to the encoded texture bytes.
         * @param size         Number of bytes in the buffer.
         * @param targetFormat Desired GPU texture format.
         * @return A TextureData object, or nullptr on failure.
         */
        static std::shared_ptr<TextureData> fromMemory(const uint8_t* data,
                                                       size_t size,
                                                       TextureFormat targetFormat);

        /**
         * @brief Returns the texture width in texels.
         */
        uint32_t getWidth() const;

        /**
         * @brief Returns the texture height in texels.
         */
        uint32_t getHeight() const;

        /**
         * @brief Returns the texture depth in texels (1 for 2D textures).
         */
        uint32_t getDepth() const;

        /**
         * @brief Returns the number of mipmap levels.
         */
        uint32_t getMipLevelCount() const;

        /**
         * @brief Returns the number of array layers.
         */
        uint32_t getArrayLayerCount() const;

        /**
         * @brief Returns the pixel/block format of the data.
         */
        TextureFormat getFormat() const;

        /**
         * @brief Returns true if the format is block-compressed.
         */
        bool isCompressed() const;

        /**
         * @brief Returns the block width in texels (1 for uncompressed).
         */
        uint32_t getBlockWidth() const;

        /**
         * @brief Returns the block height in texels (1 for uncompressed).
         */
        uint32_t getBlockHeight() const;

        /**
         * @brief Returns the number of bytes per block (or per pixel for
         * uncompressed formats).
         */
        uint32_t getBlockBytes() const;

        /**
         * @brief Returns a pointer to the raw data for a specific mip level.
         *
         * The buffer is `getDataSize(mipLevel)` bytes long and is valid for
         * the lifetime of this TextureData object.
         *
         * @param mipLevel Mipmap level index (0 = full resolution).
         */
        const void* getData(uint32_t mipLevel) const;

        /**
         * @brief Returns the size in bytes of a specific mip level's data.
         *
         * @param mipLevel Mipmap level index (0 = full resolution).
         */
        size_t getDataSize(uint32_t mipLevel) const;

        TextureData(const TextureData&)            = delete;
        TextureData& operator=(const TextureData&) = delete;

        ~TextureData();
    };

} // namespace systems::leal::campello_image
