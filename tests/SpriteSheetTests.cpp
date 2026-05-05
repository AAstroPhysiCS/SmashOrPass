#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "smashorpass/asset/SpriteSheet.hpp"

namespace {
[[nodiscard]] std::vector<uint8_t> readBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    REQUIRE(file.is_open());

    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
}

[[nodiscard]] std::vector<uint8_t> makeErrorSpriteSheetMetadata() {
    constexpr std::string_view metadata = R"json({
  "character": "error",
  "animation": "error",
  "sheet_width": 2048,
  "sheet_height": 2048,
  "frames": [
    {
      "source": "ERROR.png",
      "x_left": 0,
      "x_right": 482,
      "y_top": 0,
      "y_bottom": 482,
      "source_w": 482,
      "source_h": 482,
      "center_x": 241,
      "center_y": 241
    }
  ]
})json";

    std::vector<uint8_t> bytes;
    bytes.reserve(metadata.size());
    for (const char c : metadata) {
        bytes.push_back(static_cast<uint8_t>(c));
    }
    return bytes;
}
}  // namespace

TEST_CASE("all robot sprite sheets can be parsed") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path();
    const auto robotDir = repoRoot / "assets" / "sprites" / "characters" / "robot";

    REQUIRE(std::filesystem::exists(robotDir));

    std::vector<std::filesystem::path> metadataFiles;
    for (const auto& entry : std::filesystem::directory_iterator(robotDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().extension() == ".json") {
            metadataFiles.push_back(entry.path());
        }
    }

    REQUIRE_FALSE(metadataFiles.empty());

    std::sort(metadataFiles.begin(), metadataFiles.end());

    for (const auto& metadataPath : metadataFiles) {
        const auto stem = metadataPath.stem().string();
        const auto spritePath = metadataPath.parent_path() / (stem + ".png");
        const auto hitboxPath = metadataPath.parent_path() / (stem + "_boxes.png");

        DYNAMIC_SECTION("parse " << stem) {
            INFO("metadata: " << metadataPath.string());
            INFO("sprite sheet: " << spritePath.string());
            INFO("hitbox sheet: " << hitboxPath.string());

            REQUIRE(std::filesystem::exists(spritePath));
            REQUIRE(std::filesystem::exists(hitboxPath));

            const auto spriteBytes = readBytes(spritePath);
            const auto hitboxBytes = readBytes(hitboxPath);
            const auto metadataBytes = readBytes(metadataPath);

            REQUIRE_NOTHROW((void)sop::SpriteSheet::parse(spriteBytes, hitboxBytes, metadataBytes));
        }
    }
}

TEST_CASE("samurai idle sprite sheet can be parsed") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path();
    const auto samuraiDir = repoRoot / "assets" / "sprites" / "characters" / "samurai";
    const auto metadataPath = samuraiDir / "Idle.json";
    const auto spritePath = samuraiDir / "Idle.png";
    const auto hitboxPath = samuraiDir / "Idle_boxes.png";

    REQUIRE(std::filesystem::exists(metadataPath));
    REQUIRE(std::filesystem::exists(spritePath));
    REQUIRE(std::filesystem::exists(hitboxPath));

    const auto spriteBytes = readBytes(spritePath);
    const auto hitboxBytes = readBytes(hitboxPath);
    const auto metadataBytes = readBytes(metadataPath);

    REQUIRE_NOTHROW((void)sop::SpriteSheet::parse(spriteBytes, hitboxBytes, metadataBytes));
}

TEST_CASE("error sprite can be parsed as a one-frame fallback sprite sheet") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path();
    const auto errorSpritePath = repoRoot / "assets" / "sprites" / "ERROR.png";

    REQUIRE(std::filesystem::exists(errorSpritePath));

    const auto errorSpriteBytes = readBytes(errorSpritePath);
    const auto metadataBytes = makeErrorSpriteSheetMetadata();
    const sop::SpriteSheet spriteSheet =
        sop::SpriteSheet::parse(errorSpriteBytes, errorSpriteBytes, metadataBytes);

    const std::span<const sop::SpriteSheetFrame> frames = spriteSheet.getFrames();
    REQUIRE(frames.size() == 1);

    CHECK(frames[0].x_left == 0);
    CHECK(frames[0].x_right == 482);
    CHECK(frames[0].y_top == 0);
    CHECK(frames[0].y_bottom == 482);
    CHECK(frames[0].source_w == 482);
    CHECK(frames[0].source_h == 482);
    CHECK(frames[0].center_x == 241);
    CHECK(frames[0].center_y == 241);
}
