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
      "anchor_x": 241,
      "anchor_y": 241,
      "collision_box": {
        "x": 0,
        "y": 0,
        "width": 482,
        "height": 482
      }
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

TEST_CASE("all checked-in character sprite sheets can be parsed") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path();
    const auto charactersDir = repoRoot / "assets" / "sprites" / "characters";

    REQUIRE(std::filesystem::exists(charactersDir));

    std::vector<std::filesystem::path> metadataFiles;
    for (const auto& characterEntry : std::filesystem::directory_iterator(charactersDir)) {
        if (!characterEntry.is_directory()) {
            continue;
        }

        for (const auto& entry : std::filesystem::directory_iterator(characterEntry.path())) {
            if (!entry.is_regular_file()) {
                continue;
            }

            if (entry.path().extension() == ".json") {
                metadataFiles.push_back(entry.path());
            }
        }
    }

    REQUIRE_FALSE(metadataFiles.empty());

    std::sort(metadataFiles.begin(), metadataFiles.end());

    for (const auto& metadataPath : metadataFiles) {
        const auto stem = metadataPath.stem().string();
        const auto spritePath = metadataPath.parent_path() / (stem + ".png");
        const auto hitboxPath = metadataPath.parent_path() / (stem + "_boxes.png");
        const auto displayPath = std::filesystem::relative(metadataPath, repoRoot).string();

        DYNAMIC_SECTION("parse " << displayPath) {
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
    CHECK(frames[0].anchor_x == 241);
    CHECK(frames[0].anchor_y == 241);
    CHECK(frames[0].collision_box.x == 0.0f);
    CHECK(frames[0].collision_box.y == 0.0f);
    CHECK(frames[0].collision_box.w == 482.0f);
    CHECK(frames[0].collision_box.h == 482.0f);
}
