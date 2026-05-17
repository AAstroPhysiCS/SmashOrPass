#include "smashorpass/asset/ArenaMetadata.hpp"

#include <format>
#include <nlohmann/json.hpp>
#include <string>

namespace sop {

using namespace sop_util;

Result<ArenaMetadata> ArenaMetadata::parse(std::span<const uint8_t> metadata) {
    const auto getFloat = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<float>();
    };

    try {
        const auto json = nlohmann::json::parse(metadata.begin(), metadata.end());
        const auto& collisionsJson = json.at("collisions");
        if (!collisionsJson.is_array()) {
            return Err(std::string("Arena metadata collisions field must be an array"));
        }

        ArenaMetadata result;
        result.m_CollisionBoxes.reserve(collisionsJson.size());

        for (const auto& collisionJson : collisionsJson) {
            SDL_FRect box{
                .x = getFloat(collisionJson, "x"),
                .y = getFloat(collisionJson, "y"),
                .w = getFloat(collisionJson, "width"),
                .h = getFloat(collisionJson, "height"),
            };

            if (box.w < 0.0f || box.h < 0.0f) {
                return Err(std::string("Arena collision box cannot be negative"));
            }
            result.m_CollisionBoxes.push_back(box);
        }

        return Ok(std::move(result));
    } catch (const nlohmann::json::exception& e) {
        return Err(std::format("Failed to parse arena metadata: {}", e.what()));
    }
}

}  // namespace sop
