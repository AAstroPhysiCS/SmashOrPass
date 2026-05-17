#pragma once

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <span>
#include <vector>

#include "smashorpass/util.hpp"

namespace sop {

using namespace sop_util;

class ArenaMetadata {
   public:
    [[nodiscard]] static Result<ArenaMetadata> parse(std::span<const uint8_t> metadata);

    [[nodiscard]] std::span<const SDL_FRect> getCollisionBoxes() const {
        return m_CollisionBoxes;
    }

   private:
    std::vector<SDL_FRect> m_CollisionBoxes;
};

}  // namespace sop
