#include <campello_image/image.hpp>
#include <campello_image/texture_data.hpp>
#include <campello_image/texture_encoder.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <vector>

using namespace systems::leal::campello_image;

namespace {

// A smooth gradient with a checker on top: edges and gradients both.
std::vector<uint8_t> testImage(uint32_t w, uint32_t h) {
    std::vector<uint8_t> rgba(size_t(w) * h * 4);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t* p = &rgba[(size_t(y) * w + x) * 4];
            const bool check = ((x / 8) + (y / 8)) % 2 == 0;
            p[0] = uint8_t(x * 255 / (w - 1));
            p[1] = uint8_t(y * 255 / (h - 1));
            p[2] = check ? 200 : 40;
            p[3] = 255;
        }
    }
    return rgba;
}

double psnr(const uint8_t* a, const uint8_t* b, size_t texels) {
    double sum = 0.0;
    for (size_t i = 0; i < texels * 4; ++i) {
        if (i % 4 == 3) continue;
        const double d = double(a[i]) - double(b[i]);
        sum += d * d;
    }
    const double mse = sum / double(texels * 3);
    return mse == 0.0 ? 99.0 : 10.0 * std::log10(255.0 * 255.0 / mse);
}

} // namespace

TEST(TextureEncoder, EncodesKtx2WithMips) {
    if (!canEncode()) GTEST_SKIP() << "built without the encoder";
    const auto image = testImage(64, 64);
    const auto ktx2 = encodeKtx2(image.data(), 64, 64);
    ASSERT_FALSE(ktx2.empty());
    const uint8_t magic[12] = {0xAB, 'K', 'T', 'X', ' ', '2', '0', 0xBB, '\r', '\n', 0x1A, '\n'};
    ASSERT_GE(ktx2.size(), 12u);
    EXPECT_EQ(std::memcmp(ktx2.data(), magic, 12), 0) << "a KTX2 file";
    EXPECT_LT(ktx2.size(), image.size()) << "smaller than the pixels";

    const auto decoded = TextureData::fromMemory(ktx2.data(), ktx2.size(), TextureFormat::rgba8);
    ASSERT_NE(decoded, nullptr);
    EXPECT_EQ(decoded->getWidth(), 64u);
    EXPECT_EQ(decoded->getMipLevelCount(), 7u) << "64 down to 1";
    ASSERT_EQ(decoded->getDataSize(0), image.size());
    EXPECT_GT(psnr(image.data(), static_cast<const uint8_t*>(decoded->getData(0)), 64 * 64), 35.0) << "UASTC keeps detail";
}

TEST(TextureEncoder, CompressesForEachGpuFamily) {
    if (!canEncode()) GTEST_SKIP() << "built without the encoder";
    const auto image = testImage(64, 32);
    struct Case {
        TextureFormat format;
        uint32_t blockBytes;
    };
    for (const Case c : {Case{TextureFormat::bc7_rgba_unorm, 16}, Case{TextureFormat::astc_4x4_unorm, 16},
                         Case{TextureFormat::etc2_rgb8unorm, 8}}) {
        const auto texture = compressRGBA8(image.data(), 64, 32, c.format);
        ASSERT_NE(texture, nullptr) << int(c.format);
        EXPECT_EQ(texture->getFormat(), c.format);
        EXPECT_TRUE(texture->isCompressed());
        EXPECT_EQ(texture->getMipLevelCount(), 7u);
        EXPECT_EQ(texture->getDataSize(0), size_t(64 / 4) * (32 / 4) * c.blockBytes) << int(c.format);
    }
}

TEST(TextureEncoder, LinearDataAndOptions) {
    if (!canEncode()) GTEST_SKIP() << "built without the encoder";
    const auto image = testImage(32, 32);
    EncodeOptions options;
    options.srgb = false;  // a normal map
    options.mipmaps = false;
    options.effort = 0;
    options.threaded = false;
    const auto texture = compressRGBA8(image.data(), 32, 32, TextureFormat::rgba8, options);
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->getMipLevelCount(), 1u);
    EXPECT_GT(psnr(image.data(), static_cast<const uint8_t*>(texture->getData(0)), 32 * 32), 30.0);
}

TEST(TextureEncoder, RejectsNothingToEncode) {
    EXPECT_TRUE(encodeKtx2(nullptr, 4, 4).empty());
    const std::vector<uint8_t> one(4, 255);
    EXPECT_TRUE(encodeKtx2(one.data(), 0, 1).empty());
    EXPECT_EQ(compressRGBA8(nullptr, 4, 4, TextureFormat::bc7_rgba_unorm), nullptr);
}

// Without the encoder every encoding entry point reports failure, as
// documented, instead of crashing or returning garbage.
TEST(TextureEncoder, WithoutEncoderReturnsNothing) {
    if (canEncode()) GTEST_SKIP() << "built with the encoder";
    const auto image = testImage(8, 8);
    EXPECT_TRUE(encodeKtx2(image.data(), 8, 8).empty());
    EXPECT_EQ(compressRGBA8(image.data(), 8, 8, TextureFormat::rgba8), nullptr);
    EXPECT_TRUE(encodePng(image.data(), 8, 8).empty());
}

// encodePng(): a PNG that decodes back to the same pixels.
TEST(TextureEncoder, EncodesPng) {
    if (!canEncode()) GTEST_SKIP() << "built without the encoder";
    const uint32_t w = 37, h = 19;  // odd sizes: no row padding assumed
    std::vector<uint8_t> pixels(size_t(w) * h * 4);
    for (size_t i = 0; i < pixels.size(); ++i) pixels[i] = uint8_t(i * 31 + (i >> 5));
    for (int level : {0, 6, 10}) {
        const auto png = encodePng(pixels.data(), w, h, level);
        ASSERT_GT(png.size(), 8u);
        EXPECT_EQ(png[1], 'P');
        EXPECT_EQ(png[2], 'N');
        EXPECT_EQ(png[3], 'G');
        auto image = Image::fromMemory(png.data(), png.size());
        ASSERT_NE(image, nullptr) << "level " << level;
        ASSERT_EQ(image->getWidth(), w);
        ASSERT_EQ(image->getHeight(), h);
        const auto* back = static_cast<const uint8_t*>(image->getData());
        EXPECT_EQ(std::vector<uint8_t>(back, back + pixels.size()), pixels) << "level " << level;
    }
    EXPECT_TRUE(encodePng(nullptr, 4, 4).empty());
    EXPECT_TRUE(encodePng(pixels.data(), 0, 4).empty());
}
