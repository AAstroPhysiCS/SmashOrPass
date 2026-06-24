#pragma once

#include <SDL3/SDL_rect.h>

#include <array>
#include <cstdint>
#include <random>

#include "smashorpass/state/states/in_game/PlayerMovement.hpp"

namespace sop {

struct AgentObservation {
    SDL_FPoint SelfPosition{};
    SDL_FPoint TargetPosition{};
    PlayerActionState SelfActionState = PlayerActionState::IDLE;
    PlayerActionState TargetActionState = PlayerActionState::IDLE;
};

enum class AgentState : std::uint8_t {
    Neutral,
    Approaching,
    Pressuring,
    AttackWindup,
    AttackRecovery,
    Evading,
    Retreating,
    PerformingCombo,
};

struct AgentConfig {
    int ReactionDelayTicks = 12;
    int NeutralMinTicks = 10;
    int NeutralMaxTicks = 24;
    int PressureMinTicks = 10;
    int PressureMaxTicks = 26;
    int AttackWindupMinTicks = 7;
    int AttackWindupMaxTicks = 12;
    int AttackRecoveryTicks = 22;

    float AttackProbability = 0.9f;
    float RetreatFeintProbability = 0.05f;
    float EvadeProbability = 0.3f;
    float DistantEngageProbability = 0.50f;
    float PursuitContinueProbability = 0.72f;
    float JumpProbability = 0.42f;
    float AirDashProbability = 0.22f;
    float ComboProbability = 0.1f;
};

class Agent {
   public:
    Agent();
    explicit Agent(AgentConfig config);
    Agent(AgentConfig config, std::uint32_t seed);

    [[nodiscard]] MovementInput Tick(const AgentObservation& observation);

    void Reset();
    void SetReactionDelayTicks(int ticks);

    [[nodiscard]] int ReactionDelayTicks() const noexcept {
        return m_Config.ReactionDelayTicks;
    }

    [[nodiscard]] AgentState State() const noexcept {
        return m_State;
    }

   private:
    struct TargetSnapshot {
        SDL_FPoint Position{};
        PlayerActionState ActionState = PlayerActionState::IDLE;
    };

    enum class Combo : std::uint8_t {
        None,
        JumpDash,
        JumpDashJump,
        DashAttack,
    };

    static constexpr int MAX_REACTION_DELAY_TICKS = 60;
    static constexpr auto TARGET_HISTORY_SIZE =
        static_cast<std::size_t>(MAX_REACTION_DELAY_TICKS + 1);

    [[nodiscard]] static AgentConfig NormalizeConfig(AgentConfig config);
    [[nodiscard]] static int SideFromDistance(float distanceX, int fallbackSide) noexcept;

    [[nodiscard]] TargetSnapshot RecordTarget(const AgentObservation& observation);
    [[nodiscard]] bool Chance(float probability);
    [[nodiscard]] int RandomTicks(int minimum, int maximum);

    void EnterState(AgentState state, int durationTicks);
    void Move(int direction, MovementInput& input) const;
    void StartCombo(Combo combo, int direction);
    void TickCombo(MovementInput& input);

    AgentConfig m_Config{};
    AgentState m_State = AgentState::Neutral;

    int m_StateTicksRemaining = 0;
    int m_AttackCooldownTicks = 0;
    int m_DashCooldownTicks = 0;
    int m_JumpCooldownTicks = 0;
    int m_EvadeCooldownTicks = 0;
    int m_ComboCooldownTicks = 0;
    int m_PressureSkips = 0;
    int m_LastTargetSide = 1;
    int m_MoveDirection = 0;
    int m_PressureMoveTicksRemaining = 0;
    int m_AttackDirection = 1;
    int m_AttackFacingTicksRemaining = 0;
    int m_SeenTicks = 0;

    bool m_HistoryInitialized = false;
    bool m_PreviousThreatActive = false;

    float m_ApproachGoalX = 0.0f;

    Combo m_Combo = Combo::None;
    int m_ComboStep = 0;
    int m_ComboStepTicksRemaining = 0;
    int m_ComboDirection = 1;

    std::size_t m_TargetHistoryWriteIndex = 0;
    std::array<TargetSnapshot, TARGET_HISTORY_SIZE> m_TargetHistory{};
    std::mt19937 m_Random;
};

}  // namespace sop
