#include "smashorpass/asset/ArenaMetadata.hpp"

#include <nlohmann/json.hpp>

#include "smashorpass/core/Base.hpp"

namespace sop {

ArenaMetadata ArenaMetadata::parse(std::span<const uint8_t> metadata) {
    const auto getFloat = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<float>();
    };

    const auto json = nlohmann::json::parse(metadata.begin(), metadata.end());
    const auto& collisionsJson = json.at("collisions");

    ArenaMetadata result;
    result.m_CollisionBoxes.reserve(collisionsJson.size());

    for (const auto& collisionJson : collisionsJson) {
        SDL_FRect box{
            .x = getFloat(collisionJson, "x"),
            .y = getFloat(collisionJson, "y"),
            .w = getFloat(collisionJson, "width"),
            .h = getFloat(collisionJson, "height"),
        };

        SOP_ASSERT(box.w >= 0.0f && box.h >= 0.0f, "Arena collision box cannot be negative");
        result.m_CollisionBoxes.push_back(box);
    }

    return result;
}

}  // namespace sop
