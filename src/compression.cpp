#include <campello_image/compression.hpp>

#include <zstd.h>

#include <algorithm>

namespace systems::leal::campello_image
{

std::vector<uint8_t> zstdCompress(const void* data, size_t size, int level)
{
    if (!data && size > 0)
        return {};

    level = std::clamp(level, ZSTD_minCLevel(), ZSTD_maxCLevel());

    ZSTD_CCtx* context = ZSTD_createCCtx();
    if (!context)
        return {};

    // Record the content size and a checksum in the frame, so
    // zstdDecompress() can allocate once and detect corruption.
    ZSTD_CCtx_setParameter(context, ZSTD_c_compressionLevel, level);
    ZSTD_CCtx_setParameter(context, ZSTD_c_contentSizeFlag, 1);
    ZSTD_CCtx_setParameter(context, ZSTD_c_checksumFlag, 1);

    std::vector<uint8_t> frame(ZSTD_compressBound(size));
    const size_t written = ZSTD_compress2(context, frame.data(), frame.size(), data, size);
    ZSTD_freeCCtx(context);

    if (ZSTD_isError(written))
        return {};
    frame.resize(written);
    return frame;
}

std::optional<std::vector<uint8_t>> zstdDecompress(const void* data, size_t size, size_t maxSize)
{
    if (!data || size == 0)
        return std::nullopt;

    // Exactly one frame, nothing after it.
    const size_t frameSize = ZSTD_findFrameCompressedSize(data, size);
    if (ZSTD_isError(frameSize) || frameSize != size)
        return std::nullopt;

    const unsigned long long contentSize = ZSTD_getFrameContentSize(data, size);
    if (contentSize == ZSTD_CONTENTSIZE_UNKNOWN || contentSize == ZSTD_CONTENTSIZE_ERROR ||
        contentSize > maxSize)
        return std::nullopt;

    std::vector<uint8_t> output(static_cast<size_t>(contentSize));
    const size_t read = ZSTD_decompress(output.data(), output.size(), data, size);
    if (ZSTD_isError(read) || read != output.size())
        return std::nullopt;
    return output;
}

} // namespace systems::leal::campello_image
