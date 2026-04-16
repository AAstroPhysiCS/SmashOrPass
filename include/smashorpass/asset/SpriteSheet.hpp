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
	[[nodiscard]] static SpriteSheet parse(std::span<const uint8_t> spriteSheet, std::span<const uint8_t> hitboxSheet,
	                                       std::span<const uint8_t> metadata);

	SpriteSheet() = default;

	SpriteSheet(const SpriteSheet &) = delete;
	SpriteSheet &operator=(const SpriteSheet &) = delete;

	SpriteSheet(SpriteSheet &&other) noexcept
	    : m_SpriteSheet(other.m_SpriteSheet), m_HitboxSheet(other.m_HitboxSheet), m_Frames(std::move(other.m_Frames))
	{
		other.m_SpriteSheet = nullptr;
		other.m_HitboxSheet = nullptr;
	}

	SpriteSheet &operator=(SpriteSheet &&other) noexcept
	{
		if (this != &other) {
			if (m_SpriteSheet != nullptr)
				SDL_DestroySurface(m_SpriteSheet);
			if (m_HitboxSheet != nullptr)
				SDL_DestroySurface(m_HitboxSheet);
			m_SpriteSheet = other.m_SpriteSheet;
			m_HitboxSheet = other.m_HitboxSheet;
			m_Frames = std::move(other.m_Frames);
			other.m_SpriteSheet = nullptr;
			other.m_HitboxSheet = nullptr;
		}
		return *this;
	}

	~SpriteSheet()
	{
		if (m_SpriteSheet != nullptr)
			SDL_DestroySurface(m_SpriteSheet);
		if (m_HitboxSheet != nullptr)
			SDL_DestroySurface(m_HitboxSheet);
	}

  private:
	SDL_Surface *m_SpriteSheet = nullptr;
	SDL_Surface *m_HitboxSheet = nullptr;
	std::vector<SpriteSheetFrame> m_Frames;
};
} // namespace sop
