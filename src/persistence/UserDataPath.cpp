#include "smashorpass/persistence/UserDataPath.hpp"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>

#include <memory>

namespace sop {

Result<std::filesystem::path> UserDataPath::Get() {
    using SdlPathPtr = std::unique_ptr<char, decltype(&SDL_free)>;
    SdlPathPtr path{SDL_GetPrefPath("SmashOrPass", "SmashOrPass"), SDL_free};
    if (!path) {
        return Err(SdlError("SDL_GetPrefPath"));
    }

    return Ok(std::filesystem::path{path.get()});
}

}  // namespace sop
