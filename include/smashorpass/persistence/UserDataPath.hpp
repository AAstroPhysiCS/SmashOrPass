#pragma once

#include <filesystem>

#include "smashorpass/util.hpp"

namespace sop {

class UserDataPath {
   public:
    [[nodiscard]] static Result<std::filesystem::path> Get();
};

}  // namespace sop
