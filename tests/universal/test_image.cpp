#include <gtest/gtest.h>
#include <campello_image/image.hpp>

#include <cmath>
#include <string>

using namespace systems::leal::campello_image;

// ---------------------------------------------------------------------------
// Minimal 1×1 white PNG (RGB), generated offline and embedded as bytes.
// stb_image will decode this and force-convert to RGBA8.
// ---------------------------------------------------------------------------
static constexpr uint8_t kWhite1x1Png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, // PNG signature
    0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, // IHDR length + type
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, // width=1, height=1
    0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, // bit depth=8, RGB
    0xde, 0x00, 0x00, 0x00, 0x0c, 0x49, 0x44, 0x41, // IDAT length + type
    0x54, 0x08, 0xd7, 0x63, 0xf8, 0xcf, 0xc0, 0x00, // IDAT data (zlib)
    0x00, 0x00, 0x02, 0x00, 0x01, 0xe2, 0x21, 0xbc, // IDAT data + CRC
    0x33, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, // IEND length + type
    0x44, 0xae, 0x42, 0x60, 0x82,                   // IEND CRC
};

// ---------------------------------------------------------------------------
// Minimal 1×1 WebP (lossy, VP8), generated via Pillow and embedded as bytes.
// Decodes to a single RGBA pixel.
// ---------------------------------------------------------------------------
static constexpr uint8_t kWhite1x1Webp[] = {
    0x52, 0x49, 0x46, 0x46, 0x24, 0x00, 0x00, 0x00,
    0x57, 0x45, 0x42, 0x50, 0x56, 0x50, 0x38, 0x20,
    0x18, 0x00, 0x00, 0x00, 0x30, 0x01, 0x00, 0x9d,
    0x01, 0x2a, 0x01, 0x00, 0x01, 0x00, 0x01, 0x40,
    0x26, 0x25, 0xa4, 0x00, 0x03, 0x70, 0x00, 0xfe,
    0xfc, 0xf4, 0x00, 0x00,
};

// ---------------------------------------------------------------------------
// Helpers — shared assertions for any successfully loaded 32×32 image
// ---------------------------------------------------------------------------
static void assertValid32x32Rgba8(const std::shared_ptr<Image>& img)
{
    ASSERT_NE(img, nullptr);
    EXPECT_EQ(img->getWidth(),    32u);
    EXPECT_EQ(img->getHeight(),   32u);
    EXPECT_EQ(img->getFormat(),   ImageFormat::rgba8);
    EXPECT_EQ(img->getDataSize(), 32u * 32u * 4u);
    EXPECT_NE(img->getData(),     nullptr);
}

static void assertValid32x32Rgba32f(const std::shared_ptr<Image>& img)
{
    ASSERT_NE(img, nullptr);
    EXPECT_EQ(img->getWidth(),    32u);
    EXPECT_EQ(img->getHeight(),   32u);
    EXPECT_EQ(img->getFormat(),   ImageFormat::rgba32f);
    EXPECT_EQ(img->getDataSize(), 32u * 32u * 16u);
    EXPECT_NE(img->getData(),     nullptr);
}

// ---------------------------------------------------------------------------
// Null / invalid inputs
// ---------------------------------------------------------------------------

TEST(ImageFromFile, NullPathReturnsNullptr)
{
    EXPECT_EQ(Image::fromFile(nullptr), nullptr);
}

TEST(ImageFromFile, NonexistentFileReturnsNullptr)
{
    EXPECT_EQ(Image::fromFile("__campello_image_nonexistent_xyz__.png"), nullptr);
}

TEST(ImageFromMemory, NullDataReturnsNullptr)
{
    EXPECT_EQ(Image::fromMemory(nullptr, 0), nullptr);
}

TEST(ImageFromMemory, ZeroSizeReturnsNullptr)
{
    uint8_t dummy = 0;
    EXPECT_EQ(Image::fromMemory(&dummy, 0), nullptr);
}

TEST(ImageFromMemory, InvalidBytesReturnsNullptr)
{
    uint8_t garbage[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    EXPECT_EQ(Image::fromMemory(garbage, sizeof(garbage)), nullptr);
}

// ---------------------------------------------------------------------------
// Embedded byte-array decoding (1×1, API contract tests)
// ---------------------------------------------------------------------------

TEST(ImageFromMemory, EmbeddedPngDecodesCorrectly)
{
    auto img = Image::fromMemory(kWhite1x1Png, sizeof(kWhite1x1Png));
    ASSERT_NE(img, nullptr);
    EXPECT_EQ(img->getWidth(),    1u);
    EXPECT_EQ(img->getHeight(),   1u);
    EXPECT_EQ(img->getFormat(),   ImageFormat::rgba8);
    EXPECT_EQ(img->getDataSize(), 4u);
    EXPECT_NE(img->getData(),     nullptr);
}

TEST(ImageFromMemory, EmbeddedWebpDecodesCorrectly)
{
    auto img = Image::fromMemory(kWhite1x1Webp, sizeof(kWhite1x1Webp));
    ASSERT_NE(img, nullptr);
    EXPECT_EQ(img->getWidth(),    1u);
    EXPECT_EQ(img->getHeight(),   1u);
    EXPECT_EQ(img->getFormat(),   ImageFormat::rgba8);
    EXPECT_EQ(img->getDataSize(), 4u);
    EXPECT_NE(img->getData(),     nullptr);
}

// ---------------------------------------------------------------------------
// Real image files — 32×32 vendored test assets (tests/images/)
//
// CAMPELLO_IMAGE_TEST_ASSETS_DIR is injected by tests/CMakeLists.txt so the
// tests can locate the assets regardless of the working directory CTest uses.
// ---------------------------------------------------------------------------

#ifndef CAMPELLO_IMAGE_TEST_ASSETS_DIR
#  error "CAMPELLO_IMAGE_TEST_ASSETS_DIR must be defined by CMake"
#endif

static std::string asset(const char* filename)
{
    return std::string(CAMPELLO_IMAGE_TEST_ASSETS_DIR) + "/" + filename;
}

TEST(ImageFormats, Jpeg)
{
    assertValid32x32Rgba8(Image::fromFile(asset("test.jpg").c_str()));
}

TEST(ImageFormats, Png)
{
    assertValid32x32Rgba8(Image::fromFile(asset("test.png").c_str()));
}

TEST(ImageFormats, Bmp)
{
    assertValid32x32Rgba8(Image::fromFile(asset("test.bmp").c_str()));
}

TEST(ImageFormats, Tga)
{
    assertValid32x32Rgba8(Image::fromFile(asset("test.tga").c_str()));
}

TEST(ImageFormats, Gif)
{
    assertValid32x32Rgba8(Image::fromFile(asset("test.gif").c_str()));
}

TEST(ImageFormats, Webp)
{
    assertValid32x32Rgba8(Image::fromFile(asset("test.webp").c_str()));
}

TEST(ImageFormats, Hdr)
{
    auto img = Image::fromFile(asset("test.hdr").c_str());
    assertValid32x32Rgba32f(img);

    // Top-left pixel should be (0, 0, 0.5, 1.0)
    const auto* pixels = static_cast<const float*>(img->getData());
    EXPECT_FLOAT_EQ(pixels[0], 0.0f);
    EXPECT_FLOAT_EQ(pixels[1], 0.0f);
    EXPECT_FLOAT_EQ(pixels[2], 0.5f);
    EXPECT_FLOAT_EQ(pixels[3], 1.0f);

    // Bottom-right pixel (31,31) -> (31/8, 31/8, 0.5, 1.0)
    const size_t last = (32u * 32u - 1u) * 4u;
    EXPECT_FLOAT_EQ(pixels[last + 0], 31.0f / 8.0f);
    EXPECT_FLOAT_EQ(pixels[last + 1], 31.0f / 8.0f);
    EXPECT_FLOAT_EQ(pixels[last + 2], 0.5f);
    EXPECT_FLOAT_EQ(pixels[last + 3], 1.0f);
}

TEST(ImageFormats, Exr)
{
    auto img = Image::fromFile(asset("test.exr").c_str());
    assertValid32x32Rgba32f(img);

    // Top-left pixel should be (0, 0, 0.5, 1.0)
    const auto* pixels = static_cast<const float*>(img->getData());
    EXPECT_FLOAT_EQ(pixels[0], 0.0f);
    EXPECT_FLOAT_EQ(pixels[1], 0.0f);
    EXPECT_FLOAT_EQ(pixels[2], 0.5f);
    EXPECT_FLOAT_EQ(pixels[3], 1.0f);

    // Bottom-right pixel (31,31) -> (31/8, 31/8, 0.5, 1.0)
    const size_t last = (32u * 32u - 1u) * 4u;
    EXPECT_FLOAT_EQ(pixels[last + 0], 31.0f / 8.0f);
    EXPECT_FLOAT_EQ(pixels[last + 1], 31.0f / 8.0f);
    EXPECT_FLOAT_EQ(pixels[last + 2], 0.5f);
    EXPECT_FLOAT_EQ(pixels[last + 3], 1.0f);
}
