#pragma once

namespace systems::leal::campello_image
{
    /**
     * @brief Pixel format of decoded image data.
     *
     * campello_image decodes to a format appropriate for the source file.
     * LDR formats (JPEG, PNG, etc.) are always RGBA8. HDR formats
     * (Radiance HDR, OpenEXR) are decoded to floating-point pixel data.
     */
    enum class ImageFormat
    {
        rgba8,   ///< 4 channels, 8 bits per channel (R, G, B, A).
        rgba16f, ///< 4 channels, 16-bit float per channel (R, G, B, A).
        rgba32f, ///< 4 channels, 32-bit float per channel (R, G, B, A).
    };

} // namespace systems::leal::campello_image
