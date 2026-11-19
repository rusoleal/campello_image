#include <campello_image/texture_encoder.hpp>
#include <campello_image/texture_data.hpp>

// Kept out of the unity build (CMakeLists.txt): basisu_comp.h and
// basisu_transcoder.h in one translation unit give ambiguous names.
#if CAMPELLO_IMAGE_ENCODER
#include "basisu_comp.h"
#endif

#include <mutex>

namespace systems::leal::campello_image
{

bool canEncode()
{
#if CAMPELLO_IMAGE_ENCODER
    return true;
#else
    return false;
#endif
}

std::vector<uint8_t> encodeKtx2(const uint8_t* rgba, uint32_t width, uint32_t height, const EncodeOptions& options)
{
#if CAMPELLO_IMAGE_ENCODER
    if (!rgba || width == 0 || height == 0)
        return {};
    static std::once_flag initialized;
    std::call_once(initialized, [] { basisu::basisu_encoder_init(); });

    uint32_t flags = basisu::cFlagKTX2 | basisu::cFlagKTX2UASTCSuperCompression;
    flags |= options.effort > 4 ? 4u : options.effort;
    if (options.srgb)
        flags |= basisu::cFlagSRGB;
    if (options.mipmaps)
        flags |= options.wrap ? basisu::cFlagGenMipsWrap : basisu::cFlagGenMipsClamp;
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
    if (options.threaded)
        flags |= basisu::cFlagThreaded;
#endif
    size_t size = 0;
    void* data = basisu::basis_compress(basist::basis_tex_format::cUASTC_LDR_4x4, rgba, width, height, width, flags,
                                        0.0f, &size);
    if (!data)
        return {};
    const auto* bytes = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> out(bytes, bytes + size);
    basisu::basis_free_data(data);
    return out;
#else
    (void)rgba;
    (void)width;
    (void)height;
    (void)options;
    return {};
#endif
}

std::shared_ptr<TextureData> compressRGBA8(const uint8_t* rgba, uint32_t width, uint32_t height,
                                           TextureFormat targetFormat, const EncodeOptions& options)
{
    const auto ktx2 = encodeKtx2(rgba, width, height, options);
    if (ktx2.empty())
        return nullptr;
    return TextureData::fromMemory(ktx2.data(), ktx2.size(), targetFormat);
}

}
