#include "smashorpass/asset/SpriteSheet.hpp"

#include "smashorpass/core/Base.hpp"

#include <SDL3/SDL_iostream.h>
#include <SDL3_image/SDL_image.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <exception>
#include <limits>
#include <span>
#include <vector>

namespace {
    [[nodiscard]] uint32_t parseFrameField(const nlohmann::json& frameJson, const char* fieldName)
    {
        const auto fieldIt = frameJson.find(fieldName);
        SOP_VERIFY(fieldIt != frameJson.end(), "Sprite sheet frame is missing a required field");
        SOP_VERIFY(fieldIt->is_number_integer(), "Sprite sheet frame field must be an integer");

        const auto value = fieldIt->get<std::int64_t>();
        SOP_VERIFY(value >= 0, "Sprite sheet frame field must be non-negative");
        SOP_VERIFY(
            value <= static_cast<std::int64_t>(std::numeric_limits<uint32_t>::max()),
            "Sprite sheet frame field is out of uint32_t range"
        );

        return static_cast<uint32_t>(value);
    }
}

namespace sop {
    SpriteSheet SpriteSheet::parse(
        std::span<const uint8_t> rawImageBytes,
        std::span<const uint8_t> rawMetadataJsonBytes
    )
    {
        SOP_VERIFY(!rawImageBytes.empty(), "Sprite sheet image bytes must not be empty");
        SOP_VERIFY(!rawMetadataJsonBytes.empty(), "Sprite sheet metadata bytes must not be empty");

        nlohmann::json metadata;
        try {
            metadata = nlohmann::json::parse(rawMetadataJsonBytes.begin(), rawMetadataJsonBytes.end());
        } catch (const std::exception& exception) {
            SOP_VERIFY(false, exception.what());
        }

        SOP_VERIFY(metadata.is_object(), "Sprite sheet metadata root must be a JSON object");

        const auto framesIt = metadata.find("frames");
        SOP_VERIFY(framesIt != metadata.end(), "Sprite sheet metadata must contain a frames array");
        SOP_VERIFY(framesIt->is_array(), "Sprite sheet metadata frames field must be an array");

        std::vector<SpriteSheetFrame> frames;
        frames.reserve(framesIt->size());

        for (const auto& frameJson : *framesIt) {
            SOP_VERIFY(frameJson.is_object(), "Each sprite sheet frame must be a JSON object");

            SpriteSheetFrame frame{
                .x_left = parseFrameField(frameJson, "x_left"),
                .x_right = parseFrameField(frameJson, "x_right"),
                .y_top = parseFrameField(frameJson, "y_top"),
                .y_bottom = parseFrameField(frameJson, "y_bottom"),
                .source_w = parseFrameField(frameJson, "source_w"),
                .source_h = parseFrameField(frameJson, "source_h"),
                .center_x = parseFrameField(frameJson, "center_x"),
                .center_y = parseFrameField(frameJson, "center_y"),
            };

            SOP_VERIFY(frame.x_left <= frame.x_right, "Sprite sheet frame has invalid horizontal bounds");
            SOP_VERIFY(frame.y_top <= frame.y_bottom, "Sprite sheet frame has invalid vertical bounds");
            SOP_VERIFY(frame.source_w > 0, "Sprite sheet frame source width must be positive");
            SOP_VERIFY(frame.source_h > 0, "Sprite sheet frame source height must be positive");

            frames.push_back(frame);
        }

        auto* imageStream = SDL_IOFromConstMem(rawImageBytes.data(), rawImageBytes.size());
        SOP_SDL_ASSERT(imageStream != nullptr, "Failed to create image stream from memory");

        auto* image = IMG_Load_IO(imageStream, true);
        SOP_SDL_ASSERT(image != nullptr, "Failed to load sprite sheet image from memory");

        SpriteSheet spriteSheet;
        spriteSheet.m_Image = image;
        spriteSheet.m_Frames = std::move(frames);
        return spriteSheet;
    }
}
