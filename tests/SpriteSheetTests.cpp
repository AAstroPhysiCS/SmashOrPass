#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "smashorpass/asset/SpriteSheet.hpp"

namespace {
[[nodiscard]] std::vector<uint8_t> readBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    REQUIRE(file.is_open());

    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
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
