#pragma once

namespace systems::leal::campello_image
{
    /**
     * @brief Pixel format of decoded image data.
     *
     * campello_image always decodes to RGBA8 regardless of the source file's
     * original channel count, making it safe to upload directly to a GPU
     * texture without further conversion.
     */
    enum class ImageFormat
    {
        rgba8, ///< 4 channels, 8 bits per channel (R, G, B, A).
    };

} // namespace systems::leal::campello_image
