#pragma once

namespace sop {

    struct GameConfig {

    };

    inline static GameConfig loadDefault() {
        return GameConfig{}; 
    }

    class Game final {
    public:
        void Update();
    };
}
