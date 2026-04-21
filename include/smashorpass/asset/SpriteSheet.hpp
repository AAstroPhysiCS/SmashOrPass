#pragma once

#include <SDL3/SDL_render.h>
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

		void createSpriteTexture(SDL_Renderer *renderer);

		[[nodiscard]] SDL_Texture *getSpriteTexture() const { return m_SpriteTexture; }
		[[nodiscard]] SDL_Surface *getHitboxSurface() const { return m_HitboxSheet; }
		[[nodiscard]] std::span<const SpriteSheetFrame> getFrames() const { return m_Frames; }

		SpriteSheet(SpriteSheet &&other) noexcept
			: m_SpriteSurface(other.m_SpriteSurface), m_SpriteTexture(other.m_SpriteTexture),
			  m_HitboxSheet(other.m_HitboxSheet), m_Frames(std::move(other.m_Frames))
		{
			other.m_SpriteSurface = nullptr;
			other.m_SpriteTexture = nullptr;
			other.m_HitboxSheet = nullptr;
		}

		SpriteSheet &operator=(SpriteSheet &&other) noexcept
		{
			if (this != &other) {
				destroyOwnedResources();
				m_SpriteSurface = other.m_SpriteSurface;
				m_SpriteTexture = other.m_SpriteTexture;
				m_HitboxSheet = other.m_HitboxSheet;
				m_Frames = std::move(other.m_Frames);
				other.m_SpriteSurface = nullptr;
				other.m_SpriteTexture = nullptr;
				other.m_HitboxSheet = nullptr;
			}
			return *this;
		}

		~SpriteSheet() { destroyOwnedResources(); }

	  private:
		void destroyOwnedResources();

		SDL_Surface *m_SpriteSurface = nullptr;
		SDL_Texture *m_SpriteTexture = nullptr;
		SDL_Surface *m_HitboxSheet = nullptr;
		std::vector<SpriteSheetFrame> m_Frames;
	};
} // namespace sop
