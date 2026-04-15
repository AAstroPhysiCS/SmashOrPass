#pragma once

#include <SDL3/SDL_surface.h>

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace sop {
    struct SpriteSheetFrame {
        // Where the frame is in the spritesheet
        uint32_t x_left;
        uint32_t x_right;
        uint32_t y_top;
        uint32_t y_bottom;

        // Info about the original sprite rendered in blender
        uint32_t source_w;
        uint32_t source_h;

        // Where the center of the original sprite is now in the spritesheet
        uint32_t center_x;
        uint32_t center_y;
    };

    class SpriteSheet {
    public:
        [[nodiscard]] static SpriteSheet parse(
            std::span<const uint8_t> rawImageBytes,
            std::span<const uint8_t> rawMetadataJsonBytes
        );

        SpriteSheet() = default;

        SpriteSheet(const SpriteSheet&) = delete;
        SpriteSheet& operator=(const SpriteSheet&) = delete;

        SpriteSheet(SpriteSheet&& other) noexcept
            : m_Image(other.m_Image),
              m_Frames(std::move(other.m_Frames)) {
            other.m_Image = nullptr;
        }

        SpriteSheet& operator=(SpriteSheet&& other) noexcept {
            if (this != &other) {
                if (m_Image != nullptr) SDL_DestroySurface(m_Image);
                m_Image = other.m_Image;
                m_Frames = std::move(other.m_Frames);
                other.m_Image = nullptr;
            }
            return *this;
        }

        ~SpriteSheet() {
            if (m_Image != nullptr) SDL_DestroySurface(m_Image);
        }

    private:
        SDL_Surface* m_Image = nullptr;
        std::vector<SpriteSheetFrame> m_Frames;
    };
}
