#include <campello_image/texture_data.hpp>
#include <campello_image/image.hpp>
#include "texture_data_handle.hpp"

#include "basisu_transcoder.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace systems::leal::campello_image
{

// ---------------------------------------------------------------------------
// Basis transcoder init (once)
// ---------------------------------------------------------------------------

static void initBasisTranscoder()
{
    static bool initialized = false;
    if (!initialized)
    {
        basist::basisu_transcoder_init();
        initialized = true;
    }
}

// ---------------------------------------------------------------------------
// Format mapping: campello_image TextureFormat → basis transcoder format
// ---------------------------------------------------------------------------

static bool textureFormatToBasisFormat(TextureFormat fmt,
                                       basist::transcoder_texture_format& out)
{
    switch (fmt)
    {
        case TextureFormat::rgba8:           out = basist::transcoder_texture_format::cTFRGBA32;       break;
        case TextureFormat::bc1_rgba_unorm:  out = basist::transcoder_texture_format::cTFBC1_RGB;      break;
        case TextureFormat::bc3_rgba_unorm:  out = basist::transcoder_texture_format::cTFBC3_RGBA;     break;
        case TextureFormat::bc4_r_unorm:     out = basist::transcoder_texture_format::cTFBC4_R;        break;
        case TextureFormat::bc5_rg_unorm:    out = basist::transcoder_texture_format::cTFBC5_RG;       break;
        case TextureFormat::bc7_rgba_unorm:  out = basist::transcoder_texture_format::cTFBC7_RGBA;     break;
        case TextureFormat::etc2_rgb8unorm:  out = basist::transcoder_texture_format::cTFETC1_RGB;     break;
        case TextureFormat::eac_r11unorm:    out = basist::transcoder_texture_format::cTFETC2_EAC_R11; break;
        case TextureFormat::eac_rg11unorm:   out = basist::transcoder_texture_format::cTFETC2_EAC_RG11;break;
        case TextureFormat::astc_4x4_unorm:  out = basist::transcoder_texture_format::cTFASTC_4x4_RGBA;break;
        default:
            return false; // unsupported for Basis transcoding
    }
    return true;
}

// ---------------------------------------------------------------------------
// KTX2 identifier
// ---------------------------------------------------------------------------

static bool isKtx2(const uint8_t* data, size_t size)
{
    static const uint8_t kKtx2Id[12] = {
        0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32,
        0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
    };
    return size >= 12 && std::memcmp(data, kKtx2Id, 12) == 0;
}

// ---------------------------------------------------------------------------
// Helpers: compute mip dimensions and block counts
// ---------------------------------------------------------------------------

static uint32_t divRoundUp(uint32_t v, uint32_t d)
{
    return (v + d - 1) / d;
}

static uint32_t mipWidth(uint32_t base, uint32_t level)
{
    return std::max(1u, base >> level);
}

static uint32_t mipHeight(uint32_t base, uint32_t level)
{
    return std::max(1u, base >> level);
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

TextureData::TextureData(void* pd) : native(pd) {}

TextureData::~TextureData()
{
    if (native)
    {
        auto* h = static_cast<TextureDataHandle*>(native);
        delete h;
    }
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

uint32_t TextureData::getWidth() const
{
    return static_cast<TextureDataHandle*>(native)->width;
}

uint32_t TextureData::getHeight() const
{
    return static_cast<TextureDataHandle*>(native)->height;
}

uint32_t TextureData::getDepth() const
{
    return static_cast<TextureDataHandle*>(native)->depth;
}

uint32_t TextureData::getMipLevelCount() const
{
    return static_cast<TextureDataHandle*>(native)->mipLevels;
}

uint32_t TextureData::getArrayLayerCount() const
{
    return static_cast<TextureDataHandle*>(native)->arrayLayers;
}

TextureFormat TextureData::getFormat() const
{
    return static_cast<TextureDataHandle*>(native)->format;
}

bool TextureData::isCompressed() const
{
    return isTextureFormatCompressed(getFormat());
}

uint32_t TextureData::getBlockWidth() const
{
    return getTextureFormatBlockWidth(getFormat());
}

uint32_t TextureData::getBlockHeight() const
{
    return getTextureFormatBlockHeight(getFormat());
}

uint32_t TextureData::getBlockBytes() const
{
    return getTextureFormatBlockBytes(getFormat());
}

const void* TextureData::getData(uint32_t mipLevel) const
{
    auto* h = static_cast<TextureDataHandle*>(native);
    if (mipLevel >= h->mips.size())
        return nullptr;
    return h->mips[mipLevel].data.data();
}

size_t TextureData::getDataSize(uint32_t mipLevel) const
{
    auto* h = static_cast<TextureDataHandle*>(native);
    if (mipLevel >= h->mips.size())
        return 0;
    return h->mips[mipLevel].data.size();
}

// ---------------------------------------------------------------------------
// Factory: fromFile
// ---------------------------------------------------------------------------

std::shared_ptr<TextureData> TextureData::fromFile(const char* path,
                                                   TextureFormat targetFormat)
{
    if (!path)
        return nullptr;

    FILE* f = std::fopen(path, "rb");
    if (!f)
        return nullptr;

    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    if (size <= 0)
    {
        std::fclose(f);
        return nullptr;
    }

    std::vector<uint8_t> buf(static_cast<size_t>(size));
    const size_t read = std::fread(buf.data(), 1, buf.size(), f);
    std::fclose(f);

    if (read != buf.size())
        return nullptr;

    return fromMemory(buf.data(), buf.size(), targetFormat);
}

// ---------------------------------------------------------------------------
// Transcoding from Basis file
// ---------------------------------------------------------------------------

static std::shared_ptr<TextureData> transcodeBasis(
    const uint8_t* data, size_t size, TextureFormat targetFormat)
{
    basist::transcoder_texture_format basisFmt;
    if (!textureFormatToBasisFormat(targetFormat, basisFmt))
        return nullptr;

    initBasisTranscoder();

    basist::basisu_transcoder transcoder;
    if (!transcoder.validate_header(data, static_cast<uint32_t>(size)))
        return nullptr;

    if (!transcoder.start_transcoding(data, static_cast<uint32_t>(size)))
        return nullptr;

    uint32_t totalImages = transcoder.get_total_images(data, static_cast<uint32_t>(size));
    if (totalImages == 0)
        return nullptr;

    uint32_t totalLevels = transcoder.get_total_image_levels(
        data, static_cast<uint32_t>(size), 0);
    if (totalLevels == 0)
        return nullptr;

    auto* handle = new TextureDataHandle();
    handle->format = targetFormat;
    handle->mipLevels = totalLevels;
    handle->arrayLayers = 1;
    handle->depth = 1;
    handle->mips.resize(totalLevels);

    const uint32_t blockW = getTextureFormatBlockWidth(targetFormat);
    const uint32_t blockH = getTextureFormatBlockHeight(targetFormat);
    const uint32_t blockBytes = getTextureFormatBlockBytes(targetFormat);
    const bool isUncompressed = !isTextureFormatCompressed(targetFormat);

    for (uint32_t level = 0; level < totalLevels; ++level)
    {
        uint32_t origW = 0, origH = 0, totalBlocks = 0;
        if (!transcoder.get_image_level_desc(data, static_cast<uint32_t>(size),
                                             0, level,
                                             origW, origH, totalBlocks))
        {
            delete handle;
            return nullptr;
        }

        if (level == 0)
        {
            handle->width = origW;
            handle->height = origH;
        }

        uint32_t blocksX = divRoundUp(origW, blockW);
        uint32_t blocksY = divRoundUp(origH, blockH);

        uint32_t outputBufSize = 0;
        if (isUncompressed)
        {
            outputBufSize = origW * origH * blockBytes;
        }
        else
        {
            outputBufSize = blocksX * blocksY * blockBytes;
        }

        handle->mips[level].data.resize(outputBufSize);

        bool ok = transcoder.transcode_image_level(
            data, static_cast<uint32_t>(size),
            0, level,
            handle->mips[level].data.data(),
            isUncompressed ? (origW * origH) : (blocksX * blocksY),
            basisFmt);

        if (!ok)
        {
            delete handle;
            return nullptr;
        }
    }

    return std::shared_ptr<TextureData>(new TextureData(handle));
}

// ---------------------------------------------------------------------------
// Transcoding from KTX2 file
// ---------------------------------------------------------------------------

static std::shared_ptr<TextureData> transcodeKtx2(
    const uint8_t* data, size_t size, TextureFormat targetFormat)
{
    basist::transcoder_texture_format basisFmt;
    if (!textureFormatToBasisFormat(targetFormat, basisFmt))
        return nullptr;

    initBasisTranscoder();

    basist::ktx2_transcoder transcoder;
    if (!transcoder.init(data, static_cast<uint32_t>(size)))
        return nullptr;

    if (!transcoder.start_transcoding())
        return nullptr;

    uint32_t totalLevels = transcoder.get_levels();
    if (totalLevels == 0)
        return nullptr;

    auto* handle = new TextureDataHandle();
    handle->format = targetFormat;
    handle->width = transcoder.get_width();
    handle->height = transcoder.get_height();
    handle->depth = 1;
    handle->mipLevels = totalLevels;
    handle->arrayLayers = 1;
    handle->mips.resize(totalLevels);

    const uint32_t blockW = getTextureFormatBlockWidth(targetFormat);
    const uint32_t blockH = getTextureFormatBlockHeight(targetFormat);
    const uint32_t blockBytes = getTextureFormatBlockBytes(targetFormat);
    const bool isUncompressed = !isTextureFormatCompressed(targetFormat);

    for (uint32_t level = 0; level < totalLevels; ++level)
    {
        basist::ktx2_image_level_info levelInfo;
        if (!transcoder.get_image_level_info(levelInfo, level, 0, 0))
        {
            delete handle;
            return nullptr;
        }

        uint32_t origW = levelInfo.m_orig_width;
        uint32_t origH = levelInfo.m_orig_height;

        uint32_t blocksX = divRoundUp(origW, blockW);
        uint32_t blocksY = divRoundUp(origH, blockH);

        uint32_t outputBufSize = 0;
        if (isUncompressed)
        {
            outputBufSize = origW * origH * blockBytes;
        }
        else
        {
            outputBufSize = blocksX * blocksY * blockBytes;
        }

        handle->mips[level].data.resize(outputBufSize);

        bool ok = transcoder.transcode_image_level(
            level, 0, 0,
            handle->mips[level].data.data(),
            isUncompressed ? (origW * origH) : (blocksX * blocksY),
            basisFmt);

        if (!ok)
        {
            delete handle;
            return nullptr;
        }
    }

    return std::shared_ptr<TextureData>(new TextureData(handle));
}

// ---------------------------------------------------------------------------
// Fallback: decode via Image and copy raw pixels
// ---------------------------------------------------------------------------

static std::shared_ptr<TextureData> wrapUncompressedImage(
    std::shared_ptr<Image> img, TextureFormat targetFormat)
{
    if (!img)
        return nullptr;

    auto* handle = new TextureDataHandle();
    handle->width = img->getWidth();
    handle->height = img->getHeight();
    handle->depth = 1;
    handle->mipLevels = 1;
    handle->arrayLayers = 1;
    handle->format = targetFormat;
    handle->mips.resize(1);

    size_t dataSize = img->getDataSize();
    handle->mips[0].data.resize(dataSize);
    std::memcpy(handle->mips[0].data.data(), img->getData(), dataSize);

    return std::shared_ptr<TextureData>(new TextureData(handle));
}

// ---------------------------------------------------------------------------
// Factory: fromMemory
// ---------------------------------------------------------------------------

std::shared_ptr<TextureData> TextureData::fromMemory(const uint8_t* data,
                                                     size_t size,
                                                     TextureFormat targetFormat)
{
    if (!data || size == 0)
        return nullptr;

    // --- KTX2 --------------------------------------------------------------
    if (isKtx2(data, size))
    {
        return transcodeKtx2(data, size, targetFormat);
    }

    // --- Basis Universal (.basis) ------------------------------------------
    // Quick header validation: the first byte of a valid .basis file is 0x73
    // ('s') and the second is 0x42 ('B') in the signature.
    if (size >= 2 && data[0] == 0x73 && data[1] == 0x42)
    {
        basist::basisu_transcoder transcoder;
        if (transcoder.validate_header(data, static_cast<uint32_t>(size)))
        {
            return transcodeBasis(data, size, targetFormat);
        }
    }

    // --- Uncompressed image (PNG, JPEG, WebP, HDR, EXR, …) ----------------
    auto img = Image::fromMemory(data, size);
    if (img)
    {
        // Only rgba8 uncompressed images are supported as fallback.
        if (targetFormat != TextureFormat::rgba8)
            return nullptr;
        return wrapUncompressedImage(img, targetFormat);
    }

    return nullptr;
}

} // namespace systems::leal::campello_image
