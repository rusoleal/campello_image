#include <gtest/gtest.h>
#include <campello_image/compression.hpp>

#include <cstdint>
#include <cstring>
#include <vector>

using namespace systems::leal::campello_image;

namespace
{
    // A depth-map-like buffer: 160x120 uint16 millimetres, smooth with a
    // few holes, as campello_scanner records it.
    std::vector<uint8_t> depthLike()
    {
        std::vector<uint16_t> depth(160 * 120);
        for (size_t y = 0; y < 120; ++y)
            for (size_t x = 0; x < 160; ++x)
                depth[y * 160 + x] = (x + y) % 37 == 0 ? 0 : uint16_t(1500 + x * 3 + y);
        std::vector<uint8_t> bytes(depth.size() * 2);
        std::memcpy(bytes.data(), depth.data(), bytes.size());
        return bytes;
    }
}

TEST(Zstd, RoundTripIsLossless)
{
    const auto input = depthLike();
    const auto frame = zstdCompress(input.data(), input.size());
    ASSERT_FALSE(frame.empty());
    EXPECT_LT(frame.size(), input.size() / 2);  // smooth data compresses well

    const auto output = zstdDecompress(frame.data(), frame.size());
    ASSERT_TRUE(output.has_value());
    EXPECT_EQ(*output, input);
}

TEST(Zstd, LevelsAreClampedAndAllRoundTrip)
{
    const auto input = depthLike();
    for (int level : {-1000, -1, 1, kZstdDefaultLevel, 19, 1000})
    {
        const auto frame = zstdCompress(input.data(), input.size(), level);
        ASSERT_FALSE(frame.empty()) << "level " << level;
        const auto output = zstdDecompress(frame.data(), frame.size());
        ASSERT_TRUE(output.has_value()) << "level " << level;
        EXPECT_EQ(*output, input) << "level " << level;
    }
}

TEST(Zstd, EmptyInputRoundTrips)
{
    const auto frame = zstdCompress(nullptr, 0);
    ASSERT_FALSE(frame.empty());
    const auto output = zstdDecompress(frame.data(), frame.size());
    ASSERT_TRUE(output.has_value());
    EXPECT_TRUE(output->empty());
}

TEST(Zstd, NullDataWithSizeFails)
{
    EXPECT_TRUE(zstdCompress(nullptr, 16).empty());
    EXPECT_FALSE(zstdDecompress(nullptr, 16).has_value());
}

TEST(Zstd, RejectsGarbageTruncationAndTrailingBytes)
{
    const uint8_t garbage[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_FALSE(zstdDecompress(garbage, sizeof(garbage)).has_value());

    const auto input = depthLike();
    auto frame = zstdCompress(input.data(), input.size());
    EXPECT_FALSE(zstdDecompress(frame.data(), frame.size() - 1).has_value());

    frame.push_back(0);
    EXPECT_FALSE(zstdDecompress(frame.data(), frame.size()).has_value());
}

TEST(Zstd, DetectsCorruptionWithChecksum)
{
    const auto input = depthLike();
    auto frame = zstdCompress(input.data(), input.size());
    frame[frame.size() - 1] ^= 0xFF;  // the checksum is the last 4 bytes
    EXPECT_FALSE(zstdDecompress(frame.data(), frame.size()).has_value());
}

TEST(Zstd, RespectsMaxSize)
{
    const auto input = depthLike();
    const auto frame = zstdCompress(input.data(), input.size());
    EXPECT_FALSE(zstdDecompress(frame.data(), frame.size(), input.size() - 1).has_value());
    EXPECT_TRUE(zstdDecompress(frame.data(), frame.size(), input.size()).has_value());
}
