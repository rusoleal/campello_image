#pragma once

#include <campello_image/constants/image_format.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace systems::leal::campello_image
{
    /**
     * @brief How `Image::fromFile()` / `Image::fromMemory()` decode.
     *
     * The defaults keep the classic behaviour: every LDR source decodes to
     * RGBA8.
     */
    struct DecodeOptions
    {
        /**
         * @brief Keep 16 bits per channel for 16-bit sources (16-bit PNG).
         *
         * When `true`, a 16-bit source decodes to `ImageFormat::rgba16`
         * instead of being reduced to RGBA8: a grayscale source fills R, G
         * and B with its value and A with 65535. Needed for data images
         * such as depth maps, where 8 bits lose the measurement. 8-bit
         * sources still decode to RGBA8.
         */
        bool keep16Bit = false;
    };

    /**
     * @brief A decoded CPU-side image.
     *
     * Holds raw pixel data decoded from a file or memory buffer.
     * Supported source formats: JPEG, PNG, BMP, TGA, GIF (via stb_image),
     * WebP (via libwebp), HDR (via stb_image), and OpenEXR (via tinyexr).
     * For animated GIF files only the first frame is decoded; subsequent
     * frames are ignored.
     *
     * The pixel format depends on the source file:
     * - LDR formats (JPEG, PNG, BMP, TGA, GIF, WebP) decode to RGBA8.
     * - HDR formats (Radiance HDR, OpenEXR) decode to RGBA32F.
     *
     * Use `getFormat()` to determine the pixel format and cast `getData()`
     * appropriately:
     * - `ImageFormat::rgba8`   → `const uint8_t*`  (4 bytes per pixel)
     * - `ImageFormat::rgba16f` → `const uint16_t*` (8 bytes per pixel)
     * - `ImageFormat::rgba32f` → `const float*`    (16 bytes per pixel)
     * - `ImageFormat::rgba16`  → `const uint16_t*` (8 bytes per pixel; only
     *   with `DecodeOptions::keep16Bit`)
     *
     * Images are created via the static factory methods `fromFile` and
     * `fromMemory`. Both return `nullptr` on failure.
     *
     * **Thread safety:** Image objects are immutable after construction. All
     * getters may be called concurrently from multiple threads without
     * synchronisation. Creating or destroying distinct Image objects
     * concurrently is safe.
     *
     * @code
     * auto img = Image::fromFile("texture.png");
     * if (img) {
     *     // upload to GPU: img->getData(), img->getWidth(), img->getHeight()
     * }
     * @endcode
     */
    class Image
    {
        void* native;

        Image(void* pd);

    public:
        /**
         * @brief Decodes an image from a file path.
         *
         * Reads the file at `path` and decodes it. Returns `nullptr`
         * if the path is null, the file cannot be opened, or the format is
         * not recognised.
         *
         * @param path  Null-terminated path to the image file.
         * @return A decoded Image, or `nullptr` on failure.
         */
        static std::shared_ptr<Image> fromFile(const char* path);

        /**
         * @brief Decodes an image from a memory buffer.
         *
         * Attempts to decode `size` bytes at `data` as an image. Returns
         * `nullptr` if `data` is null, `size` is 0, or the bytes do not
         * represent a recognised image format.
         *
         * @param data  Pointer to the encoded image bytes.
         * @param size  Number of bytes in `data`.
         * @return A decoded Image, or `nullptr` on failure.
         */
        static std::shared_ptr<Image> fromMemory(const uint8_t* data, size_t size);

        /**
         * @brief Decodes an image from a file path, with `options`.
         * @see fromFile(const char*), DecodeOptions
         */
        static std::shared_ptr<Image> fromFile(const char* path, const DecodeOptions& options);

        /**
         * @brief Decodes an image from a memory buffer, with `options`.
         * @see fromMemory(const uint8_t*, size_t), DecodeOptions
         */
        static std::shared_ptr<Image> fromMemory(const uint8_t* data, size_t size,
                                                 const DecodeOptions& options);

        /**
         * @brief Returns the image width in pixels.
         */
        uint32_t getWidth() const;

        /**
         * @brief Returns the image height in pixels.
         */
        uint32_t getHeight() const;

        /**
         * @brief Returns the pixel format of the decoded data.
         */
        ImageFormat getFormat() const;

        /**
         * @brief Returns a pointer to the raw pixel data.
         *
         * The buffer is `getDataSize()` bytes long and is valid for the
         * lifetime of this Image object. The actual type depends on
         * `getFormat()`; see the `Image` class documentation for casting
         * guidelines.
         */
        const void* getData() const;

        /**
         * @brief Returns the size of the pixel data buffer in bytes.
         *
         * Equivalent to `getWidth() * getHeight() * bytesPerPixel(format)`.
         */
        size_t getDataSize() const;

        Image(const Image&)            = delete;
        Image& operator=(const Image&) = delete;

        ~Image();
    };

} // namespace systems::leal::campello_image
