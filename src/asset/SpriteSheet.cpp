#include "smashorpass/asset/SpriteSheet.hpp"

#include "smashorpass/core/Base.hpp"

#include <SDL3/SDL_iostream.h>
#include <SDL3_image/SDL_image.h>
#include <nlohmann/json.hpp>

namespace sop {
	
	SpriteSheet SpriteSheet::parse(std::span<const uint8_t> spriteSheet, std::span<const uint8_t> hitboxSheet,
								   std::span<const uint8_t> metadata)
	{
		const auto LoadSurfaceFromBytes = [](std::span<const uint8_t> bytes, const char* name) {
            SDL_IOStream* io = SDL_IOFromConstMem(bytes.data(), bytes.size());
            SOP_ASSERT(io != nullptr, "Failed to create IO stream");

            SDL_Surface* surface = IMG_Load_IO(io, true);
            SOP_ASSERT(io != nullptr, "Failed to load img");

            return surface;
        };

		const auto getU32 = [](const nlohmann::json& j, const char* key) {
			return j.at(key).get<uint32_t>();
        };

		SpriteSheet result;

		result.m_SpriteSheet = LoadSurfaceFromBytes(spriteSheet, "sprite sheet");
        result.m_HitboxSheet = LoadSurfaceFromBytes(hitboxSheet, "hitbox sheet");

		const auto json = nlohmann::json::parse(metadata.begin(), metadata.end());
		const auto& framesJson = json.at("frames");

		result.m_Frames.reserve(framesJson.size());

		for (const auto &frameJson : framesJson) {
			result.m_Frames.push_back(SpriteSheetFrame{
				.x_left = getU32(frameJson, "x_left"),
				.x_right = getU32(frameJson, "x_right"),
				.y_top = getU32(frameJson, "y_top"),
				.y_bottom = getU32(frameJson, "y_bottom"),
				.source_w = getU32(frameJson, "source_w"),
				.source_h = getU32(frameJson, "source_h"),
				.center_x = getU32(frameJson, "center_x"),
				.center_y = getU32(frameJson, "center_y"),
			});
		}

		return result;
	}
} // namespace sop
