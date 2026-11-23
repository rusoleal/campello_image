#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace systems::leal::campello_image
{
    /**
     * @brief Lossless general-purpose compression with Zstandard.
     *
     * The same Zstd that Basis Universal uses for KTX2 supercompression
     * (bundled with it, so no extra dependency), exposed for raw data that
     * isn't an image file: depth and confidence maps, recordings, caches.
     * Always available, with or without CAMPELLO_IMAGE_ENCODER.
     *
     * Each call produces or reads one complete Zstd frame (the standard
     * format: the `zstd` command-line tool reads it), with its decompressed
     * size and a checksum stored in the frame.
     *
     * **Thread safety:** both functions are stateless and may be called
     * concurrently.
     */

    /// Zstd levels: 1 (fastest) to 19 (smallest); 3 is Zstd's default.
    /// Negative levels trade ratio for even more speed.
    constexpr int kZstdDefaultLevel = 3;

    /**
     * @brief Compresses `size` bytes at `data` into one Zstd frame.
     *
     * @param level Compression level (see kZstdDefaultLevel); values out of
     *              Zstd's range are clamped.
     * @return The frame, or empty on failure (`data` null with `size` > 0).
     *         Empty input gives a valid (non-empty) frame.
     */
    std::vector<uint8_t> zstdCompress(const void* data, size_t size, int level = kZstdDefaultLevel);

    /**
     * @brief Decompresses one Zstd frame.
     *
     * The frame must record its decompressed size (zstdCompress() always
     * does), so the output is allocated once.
     *
     * @param maxSize Refuse frames that claim to decompress to more than
     *                this many bytes (protects against corrupt or hostile
     *                input asking for huge allocations).
     * @return The decompressed bytes, or std::nullopt when the input is not
     *         a valid frame, has no recorded size, exceeds `maxSize`, or
     *         fails its checksum.
     */
    std::optional<std::vector<uint8_t>> zstdDecompress(const void* data, size_t size,
                                                       size_t maxSize = size_t(1) << 31);
}
