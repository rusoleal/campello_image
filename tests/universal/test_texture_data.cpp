#include <gtest/gtest.h>
#include <campello_image/texture_data.hpp>

#include <cmath>
#include <string>

using namespace systems::leal::campello_image;

#ifndef CAMPELLO_IMAGE_TEST_ASSETS_DIR
#  error "CAMPELLO_IMAGE_TEST_ASSETS_DIR must be defined by CMake"
#endif

static std::string textureAsset(const char* filename)
{
    return std::string(CAMPELLO_IMAGE_TEST_ASSETS_DIR) + "/" + filename;
}

// ---------------------------------------------------------------------------
// Null / invalid inputs
// ---------------------------------------------------------------------------

TEST(TextureDataFromFile, NullPathReturnsNullptr)
{
    EXPECT_EQ(TextureData::fromFile(nullptr, TextureFormat::rgba8), nullptr);
}

TEST(TextureDataFromFile, NonexistentFileReturnsNullptr)
{
    EXPECT_EQ(TextureData::fromFile("__campello_image_nonexistent_xyz__.basis",
                                     TextureFormat::bc7_rgba_unorm),
              nullptr);
}

TEST(TextureDataFromMemory, NullDataReturnsNullptr)
{
    EXPECT_EQ(TextureData::fromMemory(nullptr, 0, TextureFormat::rgba8), nullptr);
}

TEST(TextureDataFromMemory, ZeroSizeReturnsNullptr)
{
    uint8_t dummy = 0;
    EXPECT_EQ(TextureData::fromMemory(&dummy, 0, TextureFormat::rgba8), nullptr);
}

TEST(TextureDataFromMemory, InvalidBytesReturnsNullptr)
{
    uint8_t garbage[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    EXPECT_EQ(TextureData::fromMemory(garbage, sizeof(garbage),
                                       TextureFormat::bc7_rgba_unorm),
              nullptr);
}

// ---------------------------------------------------------------------------
// Basis file → transcoded formats
// ---------------------------------------------------------------------------

TEST(TextureDataBasis, ToRgba8)
{
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::rgba8);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getWidth(),  8u);
    EXPECT_EQ(td->getHeight(), 8u);
    EXPECT_EQ(td->getDepth(),  1u);
    EXPECT_EQ(td->getFormat(), TextureFormat::rgba8);
    EXPECT_FALSE(td->isCompressed());
    EXPECT_EQ(td->getBlockWidth(),  1u);
    EXPECT_EQ(td->getBlockHeight(), 1u);
    EXPECT_EQ(td->getBlockBytes(),  4u);
    EXPECT_EQ(td->getMipLevelCount(), 4u);
    EXPECT_EQ(td->getArrayLayerCount(), 1u);

    // Mip 0: 8×8 RGBA8 = 256 bytes
    EXPECT_EQ(td->getDataSize(0), 8u * 8u * 4u);
    EXPECT_NE(td->getData(0), nullptr);

    // Mip 1: 4×4 RGBA8 = 64 bytes
    EXPECT_EQ(td->getDataSize(1), 4u * 4u * 4u);
    EXPECT_NE(td->getData(1), nullptr);

    // Mip 2: 2×2 RGBA8 = 16 bytes
    EXPECT_EQ(td->getDataSize(2), 2u * 2u * 4u);
    EXPECT_NE(td->getData(2), nullptr);

    // Mip 3: 1×1 RGBA8 = 4 bytes
    EXPECT_EQ(td->getDataSize(3), 1u * 1u * 4u);
    EXPECT_NE(td->getData(3), nullptr);
}

TEST(TextureDataBasis, ToBc1)
{
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::bc1_rgba_unorm);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getWidth(),  8u);
    EXPECT_EQ(td->getHeight(), 8u);
    EXPECT_EQ(td->getFormat(), TextureFormat::bc1_rgba_unorm);
    EXPECT_TRUE(td->isCompressed());
    EXPECT_EQ(td->getBlockWidth(),  4u);
    EXPECT_EQ(td->getBlockHeight(), 4u);
    EXPECT_EQ(td->getBlockBytes(),  8u);
    EXPECT_EQ(td->getMipLevelCount(), 4u);

    // Mip 0: ceil(8/4) * ceil(8/4) = 2*2 = 4 blocks * 8 bytes = 32 bytes
    EXPECT_EQ(td->getDataSize(0), 4u * 8u);

    // Mip 1: ceil(4/4) * ceil(4/4) = 1*1 = 1 block * 8 bytes = 8 bytes
    EXPECT_EQ(td->getDataSize(1), 1u * 8u);

    // Mip 2: ceil(2/4) * ceil(2/4) = 1*1 = 1 block * 8 bytes = 8 bytes
    EXPECT_EQ(td->getDataSize(2), 1u * 8u);

    // Mip 3: ceil(1/4) * ceil(1/4) = 1*1 = 1 block * 8 bytes = 8 bytes
    EXPECT_EQ(td->getDataSize(3), 1u * 8u);
}

TEST(TextureDataBasis, ToBc7)
{
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::bc7_rgba_unorm);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getFormat(), TextureFormat::bc7_rgba_unorm);
    EXPECT_TRUE(td->isCompressed());
    EXPECT_EQ(td->getBlockBytes(), 16u);

    // Mip 0: 4 blocks * 16 bytes = 64 bytes
    EXPECT_EQ(td->getDataSize(0), 4u * 16u);
}

TEST(TextureDataBasis, ToEtc2)
{
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::etc2_rgb8unorm);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getFormat(), TextureFormat::etc2_rgb8unorm);
    EXPECT_TRUE(td->isCompressed());
    EXPECT_EQ(td->getBlockBytes(), 8u);

    // Mip 0: 4 blocks * 8 bytes = 32 bytes
    EXPECT_EQ(td->getDataSize(0), 4u * 8u);
}

TEST(TextureDataBasis, ToAstc4x4)
{
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::astc_4x4_unorm);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getFormat(), TextureFormat::astc_4x4_unorm);
    EXPECT_TRUE(td->isCompressed());
    EXPECT_EQ(td->getBlockBytes(), 16u);

    // Mip 0: 4 blocks * 16 bytes = 64 bytes
    EXPECT_EQ(td->getDataSize(0), 4u * 16u);
}

// ---------------------------------------------------------------------------
// KTX2 file → transcoded formats
// ---------------------------------------------------------------------------

TEST(TextureDataKtx2, ToRgba8)
{
    auto td = TextureData::fromFile(textureAsset("test.ktx2").c_str(),
                                    TextureFormat::rgba8);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getWidth(),  8u);
    EXPECT_EQ(td->getHeight(), 8u);
    EXPECT_EQ(td->getFormat(), TextureFormat::rgba8);
    EXPECT_FALSE(td->isCompressed());
    EXPECT_EQ(td->getMipLevelCount(), 4u);
    EXPECT_EQ(td->getDataSize(0), 8u * 8u * 4u);
}

TEST(TextureDataKtx2, ToBc7)
{
    auto td = TextureData::fromFile(textureAsset("test.ktx2").c_str(),
                                    TextureFormat::bc7_rgba_unorm);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getFormat(), TextureFormat::bc7_rgba_unorm);
    EXPECT_TRUE(td->isCompressed());
    EXPECT_EQ(td->getMipLevelCount(), 4u);
    EXPECT_EQ(td->getDataSize(0), 4u * 16u);
}

TEST(TextureDataKtx2, ToAstc4x4)
{
    auto td = TextureData::fromFile(textureAsset("test.ktx2").c_str(),
                                    TextureFormat::astc_4x4_unorm);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getFormat(), TextureFormat::astc_4x4_unorm);
    EXPECT_TRUE(td->isCompressed());
    EXPECT_EQ(td->getMipLevelCount(), 4u);
    EXPECT_EQ(td->getDataSize(0), 4u * 16u);
}

// ---------------------------------------------------------------------------
// Uncompressed image fallback (PNG → TextureData)
// ---------------------------------------------------------------------------

TEST(TextureDataUncompressed, PngToRgba8)
{
    auto td = TextureData::fromFile(textureAsset("test.png").c_str(),
                                    TextureFormat::rgba8);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getWidth(),  32u);
    EXPECT_EQ(td->getHeight(), 32u);
    EXPECT_EQ(td->getFormat(), TextureFormat::rgba8);
    EXPECT_FALSE(td->isCompressed());
    EXPECT_EQ(td->getMipLevelCount(), 1u);
    EXPECT_EQ(td->getDataSize(0), 32u * 32u * 4u);
}

TEST(TextureDataUncompressed, PngToBc7Fails)
{
    // Uncompressed images cannot be transcoded to block-compressed formats
    // via the current fallback path.
    auto td = TextureData::fromFile(textureAsset("test.png").c_str(),
                                    TextureFormat::bc7_rgba_unorm);
    EXPECT_EQ(td, nullptr);
}

// ---------------------------------------------------------------------------
// Unsupported transcoding target
// ---------------------------------------------------------------------------

TEST(TextureDataBasis, UnsupportedFormatReturnsNullptr)
{
    // BC6H is not a valid Basis Universal transcoding target.
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::bc6h_rgb_ufloat);
    EXPECT_EQ(td, nullptr);
}

// ---------------------------------------------------------------------------
// Data out-of-range access
// ---------------------------------------------------------------------------

TEST(TextureDataAccess, OutOfRangeMipReturnsNull)
{
    auto td = TextureData::fromFile(textureAsset("test.basis").c_str(),
                                    TextureFormat::rgba8);
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->getData(999), nullptr);
    EXPECT_EQ(td->getDataSize(999), 0u);
}

// ---------------------------------------------------------------------------
// Value alignment with campello_gpu PixelFormat
// ---------------------------------------------------------------------------

TEST(TextureFormatValues, AlignWithCampelloGpuPixelFormat)
{
    // These values must stay in sync with campello_gpu::PixelFormat.
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::rgba8),    70u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::rgba16f),  115u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::rgba32f),  125u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::bc1_rgba_unorm),  130u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::bc3_rgba_unorm),  134u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::bc4_r_unorm),     140u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::bc5_rg_unorm),    142u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::bc6h_rgb_ufloat), 151u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::bc7_rgba_unorm),  152u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::eac_r11unorm),    170u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::etc2_rgb8unorm),  180u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::etc2_rgb8a1unorm),182u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::eac_rg11unorm),   174u);
    EXPECT_EQ(static_cast<uint32_t>(TextureFormat::astc_4x4_unorm),  186u);
}
