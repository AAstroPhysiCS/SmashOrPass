#pragma once

#include <chrono>
#include "smashorpass/core/Game.hpp"
#include "smashorpass/state/State.hpp"
#include "smashorpass/ui/GameScreen.hpp"
#include "smashorpass/ui/PauseScreen.hpp"

namespace sop {

class InGameState final : public State {
   public:
    explicit InGameState(AppCtx& ctx);
    ~InGameState() override = default;

    [[nodiscard]] std::string_view DebugName() const final {
        return "InGame";
    }

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

   private:
    void ResetFrameTimer();
    void TogglePause();

    Game m_Game;
    GameScreen m_GameScreen;
    PauseScreen m_PauseScreen;

    using Clock = std::chrono::steady_clock;
    Clock::time_point m_PreviousUpdateTime;
    Clock::time_point m_PreviousGameLogicTick;
    Clock::time_point m_PreviousAnimationTick;
    bool m_Paused = false;
};

}  // namespace sop
