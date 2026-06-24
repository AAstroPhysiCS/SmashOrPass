#include "smashorpass/state/states/in_game/AiAgent.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <utility>

namespace sop {

constexpr float MOVE_DEAD_ZONE = 4.0f;
constexpr float PREFERRED_MIN_RANGE = 72.0f;
constexpr float PREFERRED_MAX_RANGE = 112.0f;
constexpr float ATTACK_RANGE = 140.0f;
constexpr float ATTACK_VERTICAL_RANGE = 85.0f;
constexpr float REENGAGE_RANGE = 170.0f;
constexpr float RETREAT_STOP_RANGE = 165.0f;
constexpr float THREAT_RANGE = 155.0f;
constexpr float THREAT_VERTICAL_RANGE = 100.0f;
constexpr float DISTANT_INTEREST_RANGE = 245.0f;
constexpr float FAR_PURSUIT_RANGE = 330.0f;
constexpr float APPROACH_DASH_RANGE = 220.0f;
constexpr float APPROACH_GOAL_MIN_RANGE = 90.0f;
constexpr float APPROACH_GOAL_MAX_RANGE = 120.0f;
constexpr float APPROACH_GOAL_DEAD_ZONE = 8.0f;
constexpr float JUMP_VERTICAL_SEPARATION = 52.0f;
constexpr float JUMP_MAX_HORIZONTAL_RANGE = 210.0f;
constexpr float AERIAL_MIN_HORIZONTAL_RANGE = 35.0f;
constexpr float AERIAL_MAX_HORIZONTAL_RANGE = 220.0f;
constexpr float AERIAL_MAX_VERTICAL_RANGE = 125.0f;
constexpr float DASH_ATTACK_MIN_RANGE = 100.0f;
constexpr float DASH_ATTACK_MAX_RANGE = 180.0f;

constexpr int ATTACK_COOLDOWN_TICKS = 54;
constexpr int DASH_COOLDOWN_TICKS = 72;
constexpr int JUMP_COOLDOWN_TICKS = 84;
constexpr int EVADE_COOLDOWN_TICKS = 90;
constexpr int COMBO_COOLDOWN_TICKS = 180;
constexpr int APPROACH_REPLAN_MIN_TICKS = 54;
constexpr int APPROACH_REPLAN_MAX_TICKS = 78;
constexpr int RETREAT_MIN_TICKS = 24;
constexpr int RETREAT_MAX_TICKS = 42;
constexpr int EVADE_MIN_TICKS = 22;
constexpr int EVADE_MAX_TICKS = 34;
constexpr int DISTANT_OBSERVE_EXTRA_MIN_TICKS = 18;
constexpr int DISTANT_OBSERVE_EXTRA_MAX_TICKS = 42;
constexpr int PRESSURE_MOVE_MIN_TICKS = 6;
constexpr int PRESSURE_MOVE_MAX_TICKS = 14;
constexpr int ATTACK_FACE_TICKS = 3;
constexpr int COMBO_EXIT_MIN_TICKS = 8;
constexpr int COMBO_EXIT_MAX_TICKS = 18;
constexpr int JUMP_TO_DASH_MIN_TICKS = 4;
constexpr int JUMP_TO_DASH_MAX_TICKS = 7;
constexpr int DASH_TO_DOUBLE_JUMP_MIN_TICKS = 5;
constexpr int DASH_TO_DOUBLE_JUMP_MAX_TICKS = 8;
constexpr int DASH_ATTACK_DELAY_MIN_TICKS = 8;
constexpr int DASH_ATTACK_DELAY_MAX_TICKS = 12;

constexpr float APPROACH_DASH_PROBABILITY = 0.38f;
constexpr float LEVEL_APPROACH_JUMP_MULTIPLIER = 0.55f;
constexpr float PRESSURE_JUMP_MULTIPLIER = 0.85f;
constexpr float LEVEL_PRESSURE_JUMP_MULTIPLIER = 0.50f;
constexpr float ATTACK_READY_AIR_DASH_MULTIPLIER = 0.45f;
constexpr float ELEVATED_AIR_DASH_MULTIPLIER = 1.35f;
constexpr float DOUBLE_JUMP_MULTIPLIER = 2.25f;
constexpr float EVADE_AIR_DASH_PROBABILITY = 0.28f;
constexpr float EVADE_DASH_PROBABILITY = 0.62f;
constexpr float EVADE_JUMP_PROBABILITY = 0.34f;
constexpr float PRESSURE_RANGE_CORRECTION_PROBABILITY = 0.68f;
constexpr float PRESSURE_MICRO_STEP_PROBABILITY = 0.16f;
constexpr float RETREAT_WHEN_ATTACK_UNAVAILABLE_PROBABILITY = 0.30f;
constexpr float RETREAT_AFTER_ATTACK_PROBABILITY = 0.30f;
constexpr float CLOSE_RETREAT_AFTER_ATTACK_PROBABILITY = 0.68f;
constexpr float POST_ATTACK_OBSERVE_PROBABILITY = 0.15f;
constexpr float FAR_PURSUIT_MULTIPLIER = 0.55f;
constexpr float ATTACK_PROBABILITY_STEP = 0.12f;
constexpr int MAX_PRESSURE_SKIPS = 1;

void TickDown(int& ticks) noexcept {
    ticks = std::max(0, ticks - 1);
}

float Clamp01(float value) noexcept {
    return std::clamp(value, 0.0f, 1.0f);
}

Agent::Agent() : Agent(AgentConfig{}, static_cast<std::uint32_t>(std::random_device{}())) {}

Agent::Agent(AgentConfig config)
    : Agent(config, static_cast<std::uint32_t>(std::random_device{}())) {}

Agent::Agent(AgentConfig config, std::uint32_t seed)
    : m_Config(NormalizeConfig(config)), m_Random(seed) {
    Reset();
}

AgentConfig Agent::NormalizeConfig(AgentConfig config) {
    config.ReactionDelayTicks = std::clamp(config.ReactionDelayTicks, 0, MAX_REACTION_DELAY_TICKS);

    auto normalizeRange = [](int& minimum, int& maximum) {
        minimum = std::max(0, minimum);
        maximum = std::max(minimum, maximum);
    };
    normalizeRange(config.NeutralMinTicks, config.NeutralMaxTicks);
    normalizeRange(config.PressureMinTicks, config.PressureMaxTicks);
    normalizeRange(config.AttackWindupMinTicks, config.AttackWindupMaxTicks);
    config.AttackRecoveryTicks = std::max(0, config.AttackRecoveryTicks);

    config.AttackProbability = Clamp01(config.AttackProbability);
    config.RetreatFeintProbability = Clamp01(config.RetreatFeintProbability);
    config.EvadeProbability = Clamp01(config.EvadeProbability);
    config.DistantEngageProbability = Clamp01(config.DistantEngageProbability);
    config.PursuitContinueProbability = Clamp01(config.PursuitContinueProbability);
    config.JumpProbability = Clamp01(config.JumpProbability);
    config.AirDashProbability = Clamp01(config.AirDashProbability);
    config.ComboProbability = Clamp01(config.ComboProbability);
    return config;
}

int Agent::SideFromDistance(float distanceX, int fallbackSide) noexcept {
    if (distanceX > MOVE_DEAD_ZONE) {
        return 1;
    }
    if (distanceX < -MOVE_DEAD_ZONE) {
        return -1;
    }
    return fallbackSide < 0 ? -1 : 1;
}

void Agent::Reset() {
    m_State = AgentState::Neutral;
    m_StateTicksRemaining = m_Config.ReactionDelayTicks +
                            RandomTicks(m_Config.NeutralMinTicks, m_Config.NeutralMaxTicks);
    m_AttackCooldownTicks = 0;
    m_DashCooldownTicks = 0;
    m_JumpCooldownTicks = 0;
    m_EvadeCooldownTicks = 0;
    m_ComboCooldownTicks = 0;
    m_PressureSkips = 0;
    m_LastTargetSide = Chance(0.5f) ? 1 : -1;
    m_MoveDirection = 0;
    m_PressureMoveTicksRemaining = 0;
    m_AttackDirection = m_LastTargetSide;
    m_AttackFacingTicksRemaining = 0;
    m_SeenTicks = 0;
    m_HistoryInitialized = false;
    m_PreviousThreatActive = false;
    m_ApproachGoalX = 0.0f;
    m_Combo = Combo::None;
    m_ComboStep = 0;
    m_ComboStepTicksRemaining = 0;
    m_ComboDirection = m_LastTargetSide;
    m_TargetHistoryWriteIndex = 0;
}

void Agent::SetReactionDelayTicks(int ticks) {
    m_Config.ReactionDelayTicks = std::clamp(ticks, 0, MAX_REACTION_DELAY_TICKS);
    m_SeenTicks = 0;
    m_PreviousThreatActive = false;
}

Agent::TargetSnapshot Agent::RecordTarget(const AgentObservation& observation) {
    const TargetSnapshot currentTarget{observation.TargetPosition, observation.TargetActionState};

    if (!m_HistoryInitialized) {
        m_TargetHistory.fill(currentTarget);
        m_TargetHistoryWriteIndex = 0;
        m_HistoryInitialized = true;
    } else {
        m_TargetHistoryWriteIndex = (m_TargetHistoryWriteIndex + 1) % TARGET_HISTORY_SIZE;
        m_TargetHistory[m_TargetHistoryWriteIndex] = currentTarget;
    }

    m_SeenTicks = std::min(m_SeenTicks + 1, MAX_REACTION_DELAY_TICKS + 1);
    const auto delay = static_cast<std::size_t>(m_Config.ReactionDelayTicks);
    const auto delayedIndex =
        (m_TargetHistoryWriteIndex + TARGET_HISTORY_SIZE - delay) % TARGET_HISTORY_SIZE;
    return m_TargetHistory[delayedIndex];
}

bool Agent::Chance(float probability) {
    probability = Clamp01(probability);
    if (probability == 0.0f) {
        return false;
    }
    if (probability == 1.0f) {
        return true;
    }
    return std::bernoulli_distribution{probability}(m_Random);
}

int Agent::RandomTicks(int minimum, int maximum) {
    return std::uniform_int_distribution{minimum, maximum}(m_Random);
}

void Agent::EnterState(AgentState state, int durationTicks) {
    m_State = state;
    m_StateTicksRemaining = std::max(0, durationTicks);
}

void Agent::Move(int direction, MovementInput& input) const {
    if (direction < 0) {
        input.MoveLeftHeld = true;
    } else if (direction > 0) {
        input.MoveRightHeld = true;
    }
}

void Agent::StartCombo(Combo combo, int direction) {
    m_Combo = combo;
    m_ComboStep = 0;
    m_ComboStepTicksRemaining = 0;
    m_ComboDirection = direction < 0 ? -1 : 1;
    m_ComboCooldownTicks = COMBO_COOLDOWN_TICKS;
    m_MoveDirection = 0;
    m_PressureMoveTicksRemaining = 0;

    if (combo == Combo::JumpDash || combo == Combo::JumpDashJump) {
        m_JumpCooldownTicks = JUMP_COOLDOWN_TICKS;
        m_DashCooldownTicks = DASH_COOLDOWN_TICKS;
    } else if (combo == Combo::DashAttack) {
        m_DashCooldownTicks = DASH_COOLDOWN_TICKS;
    }

    EnterState(AgentState::PerformingCombo, 0);
}

void Agent::TickCombo(MovementInput& input) {
    if (m_ComboStepTicksRemaining > 0) {
        --m_ComboStepTicksRemaining;
        Move(m_ComboDirection, input);
        return;
    }

    switch (m_Combo) {
        case Combo::JumpDash:
            if (m_ComboStep == 0) {
                Move(m_ComboDirection, input);
                input.JumpPressed = true;
                m_ComboStep = 1;
                m_ComboStepTicksRemaining =
                    RandomTicks(JUMP_TO_DASH_MIN_TICKS, JUMP_TO_DASH_MAX_TICKS);
                return;
            }
            Move(m_ComboDirection, input);
            input.DashPressed = true;
            break;

        case Combo::JumpDashJump:
            if (m_ComboStep == 0) {
                Move(m_ComboDirection, input);
                input.JumpPressed = true;
                m_ComboStep = 1;
                m_ComboStepTicksRemaining =
                    RandomTicks(JUMP_TO_DASH_MIN_TICKS, JUMP_TO_DASH_MAX_TICKS);
                return;
            }
            if (m_ComboStep == 1) {
                Move(m_ComboDirection, input);
                input.DashPressed = true;
                m_ComboStep = 2;
                m_ComboStepTicksRemaining =
                    RandomTicks(DASH_TO_DOUBLE_JUMP_MIN_TICKS, DASH_TO_DOUBLE_JUMP_MAX_TICKS);
                return;
            }
            Move(m_ComboDirection, input);
            input.JumpPressed = true;
            break;

        case Combo::DashAttack:
            if (m_ComboStep == 0) {
                Move(m_ComboDirection, input);
                input.DashPressed = true;
                m_ComboStep = 1;
                m_ComboStepTicksRemaining =
                    RandomTicks(DASH_ATTACK_DELAY_MIN_TICKS, DASH_ATTACK_DELAY_MAX_TICKS);
                return;
            }
            Move(m_ComboDirection, input);
            input.AttackPressed = true;
            input.AttackHeld = true;
            m_AttackCooldownTicks = ATTACK_COOLDOWN_TICKS;
            m_Combo = Combo::None;
            EnterState(AgentState::AttackRecovery, m_Config.AttackRecoveryTicks);
            return;

        case Combo::None:
            EnterState(AgentState::Neutral, 1);
            return;
    }

    m_Combo = Combo::None;
    EnterState(AgentState::Neutral, RandomTicks(COMBO_EXIT_MIN_TICKS, COMBO_EXIT_MAX_TICKS));
}

MovementInput Agent::Tick(const AgentObservation& observation) {
    TickDown(m_StateTicksRemaining);
    TickDown(m_AttackCooldownTicks);
    TickDown(m_DashCooldownTicks);
    TickDown(m_JumpCooldownTicks);
    TickDown(m_EvadeCooldownTicks);
    TickDown(m_ComboCooldownTicks);
    TickDown(m_PressureMoveTicksRemaining);

    const TargetSnapshot target = RecordTarget(observation);
    const float distanceX = target.Position.x - observation.SelfPosition.x;
    const float distanceY = target.Position.y - observation.SelfPosition.y;
    const float currentDistanceX = observation.TargetPosition.x - observation.SelfPosition.x;
    const float absX = std::abs(distanceX);
    const float absY = std::abs(distanceY);

    if (distanceX > MOVE_DEAD_ZONE) {
        m_LastTargetSide = 1;
    } else if (distanceX < -MOVE_DEAD_ZONE) {
        m_LastTargetSide = -1;
    }

    const int delayedSide = SideFromDistance(distanceX, m_LastTargetSide);
    const int currentSide = SideFromDistance(currentDistanceX, delayedSide);
    const bool canAct = observation.SelfActionState == PlayerActionState::IDLE;
    const bool selfAttacking = observation.SelfActionState == PlayerActionState::ATTACKING;
    const bool targetElevated = distanceY <= -JUMP_VERTICAL_SEPARATION;
    MovementInput input{};

    const bool threatActive = target.ActionState == PlayerActionState::ATTACKING &&
                              absX <= THREAT_RANGE && absY <= THREAT_VERTICAL_RANGE;
    const bool perceptionReady = m_SeenTicks > m_Config.ReactionDelayTicks;
    const bool threatStarted =
        perceptionReady && threatActive && !std::exchange(m_PreviousThreatActive, threatActive);
    if (!perceptionReady) {
        m_PreviousThreatActive = false;
    }

    auto randomTicks = [&](int minimum, int maximum) { return RandomTicks(minimum, maximum); };

    auto beginNeutral = [&] {
        EnterState(AgentState::Neutral,
                   randomTicks(m_Config.NeutralMinTicks, m_Config.NeutralMaxTicks));
        m_MoveDirection = 0;
        m_PressureMoveTicksRemaining = 0;
    };

    auto planApproach = [&] {
        const float maxUsefulRange = std::max(
            PREFERRED_MIN_RANGE, std::min(APPROACH_GOAL_MAX_RANGE, absX - APPROACH_GOAL_DEAD_ZONE));
        const float minUsefulRange = std::min(APPROACH_GOAL_MIN_RANGE, maxUsefulRange);
        const float desiredRange =
            std::uniform_real_distribution{minUsefulRange, maxUsefulRange}(m_Random);

        m_ApproachGoalX = target.Position.x - static_cast<float>(delayedSide) * desiredRange;
        m_MoveDirection = delayedSide;
    };

    auto useApproachMobility = [&] {
        if (!canAct) {
            return;
        }

        const bool hopRange =
            absX <= JUMP_MAX_HORIZONTAL_RANGE && absY <= AERIAL_MAX_VERTICAL_RANGE;
        if (targetElevated && hopRange && m_JumpCooldownTicks == 0 &&
            Chance(m_Config.JumpProbability)) {
            input.JumpPressed = true;
            m_JumpCooldownTicks = JUMP_COOLDOWN_TICKS;
        } else if (absX >= APPROACH_DASH_RANGE && m_DashCooldownTicks == 0 &&
                   Chance(APPROACH_DASH_PROBABILITY)) {
            input.DashPressed = true;
            m_DashCooldownTicks = DASH_COOLDOWN_TICKS;
        } else if (hopRange && m_JumpCooldownTicks == 0 &&
                   Chance(m_Config.JumpProbability * LEVEL_APPROACH_JUMP_MULTIPLIER)) {
            input.JumpPressed = true;
            m_JumpCooldownTicks = JUMP_COOLDOWN_TICKS;
        }
    };

    auto beginApproach = [&] {
        if (absX > REENGAGE_RANGE) {
            m_PressureSkips = 0;
        }
        planApproach();
        EnterState(AgentState::Approaching,
                   randomTicks(APPROACH_REPLAN_MIN_TICKS, APPROACH_REPLAN_MAX_TICKS));
        Move(m_MoveDirection, input);
        useApproachMobility();
    };

    auto planPressureMovement = [&] {
        m_MoveDirection = 0;
        m_PressureMoveTicksRemaining = 0;

        if (absX > PREFERRED_MAX_RANGE) {
            m_MoveDirection = Chance(PRESSURE_RANGE_CORRECTION_PROBABILITY) ? delayedSide : 0;
        } else if (absX < PREFERRED_MIN_RANGE) {
            m_MoveDirection = Chance(PRESSURE_RANGE_CORRECTION_PROBABILITY) ? -delayedSide : 0;
        } else if (Chance(PRESSURE_MICRO_STEP_PROBABILITY)) {
            m_MoveDirection = Chance(0.5f) ? delayedSide : -delayedSide;
        }

        if (m_MoveDirection != 0) {
            m_PressureMoveTicksRemaining =
                randomTicks(PRESSURE_MOVE_MIN_TICKS, PRESSURE_MOVE_MAX_TICKS);
        }
    };

    auto beginPressure = [&] {
        EnterState(AgentState::Pressuring,
                   randomTicks(m_Config.PressureMinTicks, m_Config.PressureMaxTicks));
        planPressureMovement();
    };

    auto beginRetreat = [&] {
        EnterState(AgentState::Retreating, randomTicks(RETREAT_MIN_TICKS, RETREAT_MAX_TICKS));
        m_MoveDirection = -delayedSide;
        m_PressureMoveTicksRemaining = 0;
        Move(m_MoveDirection, input);
    };

    auto beginAttack = [&] {
        m_AttackDirection = currentSide;
        m_AttackFacingTicksRemaining = ATTACK_FACE_TICKS;
        m_MoveDirection = 0;
        m_PressureMoveTicksRemaining = 0;
        EnterState(AgentState::AttackWindup,
                   randomTicks(m_Config.AttackWindupMinTicks, m_Config.AttackWindupMaxTicks));
    };

    auto aerialDirection = [&] {
        if (absX < PREFERRED_MIN_RANGE) {
            return -currentSide;
        }
        if (absX > PREFERRED_MAX_RANGE) {
            return currentSide;
        }
        return Chance(0.65f) ? currentSide : -currentSide;
    };

    auto tryAerialDash = [&](float multiplier) {
        const bool available = canAct && m_ComboCooldownTicks == 0 && m_DashCooldownTicks == 0 &&
                               m_JumpCooldownTicks == 0;
        const bool inRange = absX >= AERIAL_MIN_HORIZONTAL_RANGE &&
                             absX <= AERIAL_MAX_HORIZONTAL_RANGE &&
                             absY <= AERIAL_MAX_VERTICAL_RANGE;
        if (!available || !inRange) {
            return false;
        }

        float probability = m_Config.AirDashProbability * multiplier;
        if (targetElevated) {
            probability *= ELEVATED_AIR_DASH_MULTIPLIER;
        }
        if (!Chance(probability)) {
            return false;
        }

        const float doubleJumpProbability =
            m_Config.ComboProbability * (targetElevated ? DOUBLE_JUMP_MULTIPLIER : 0.5f);
        StartCombo(Chance(doubleJumpProbability) ? Combo::JumpDashJump : Combo::JumpDash,
                   aerialDirection());
        return true;
    };

    auto tryDashAttack = [&] {
        const bool available = canAct && m_ComboCooldownTicks == 0 && m_DashCooldownTicks == 0 &&
                               m_AttackCooldownTicks == 0;
        const bool inRange = absX >= DASH_ATTACK_MIN_RANGE && absX <= DASH_ATTACK_MAX_RANGE &&
                             absY <= ATTACK_VERTICAL_RANGE;
        if (!available || !inRange || !Chance(m_Config.ComboProbability)) {
            return false;
        }

        m_PressureSkips = 0;
        StartCombo(Combo::DashAttack, currentSide);
        return true;
    };

    auto tryPressureJump = [&](float multiplier) {
        const bool available = canAct && m_JumpCooldownTicks == 0;
        const bool inRange = absX <= JUMP_MAX_HORIZONTAL_RANGE && absY <= AERIAL_MAX_VERTICAL_RANGE;
        if (!available || !inRange) {
            return false;
        }

        const float contextMultiplier =
            targetElevated ? PRESSURE_JUMP_MULTIPLIER : LEVEL_PRESSURE_JUMP_MULTIPLIER;
        if (!Chance(m_Config.JumpProbability * contextMultiplier * multiplier)) {
            return false;
        }

        Move(aerialDirection(), input);
        input.JumpPressed = true;
        m_JumpCooldownTicks = JUMP_COOLDOWN_TICKS;
        return true;
    };

    if (selfAttacking) {
        m_Combo = Combo::None;
        if (m_State != AgentState::AttackRecovery) {
            EnterState(AgentState::AttackRecovery, m_Config.AttackRecoveryTicks);
        }
        return input;
    }

    const bool canEvade = m_State == AgentState::Neutral || m_State == AgentState::Approaching ||
                          m_State == AgentState::Pressuring;
    if (threatStarted && canAct && canEvade && m_EvadeCooldownTicks == 0 &&
        Chance(m_Config.EvadeProbability)) {
        m_EvadeCooldownTicks = EVADE_COOLDOWN_TICKS;
        m_PressureSkips = 0;
        m_MoveDirection = -delayedSide;
        m_PressureMoveTicksRemaining = 0;

        if (m_ComboCooldownTicks == 0 && m_DashCooldownTicks == 0 && m_JumpCooldownTicks == 0 &&
            Chance(EVADE_AIR_DASH_PROBABILITY)) {
            StartCombo(Combo::JumpDash, m_MoveDirection);
            TickCombo(input);
            return input;
        }

        EnterState(AgentState::Evading, randomTicks(EVADE_MIN_TICKS, EVADE_MAX_TICKS));
        Move(m_MoveDirection, input);
        if (m_DashCooldownTicks == 0 && Chance(EVADE_DASH_PROBABILITY)) {
            input.DashPressed = true;
            m_DashCooldownTicks = DASH_COOLDOWN_TICKS;
        } else if (m_JumpCooldownTicks == 0 && Chance(EVADE_JUMP_PROBABILITY)) {
            input.JumpPressed = true;
            m_JumpCooldownTicks = JUMP_COOLDOWN_TICKS;
        }
        return input;
    }

    for (int transitions = 0; transitions < 6; ++transitions) {
        switch (m_State) {
            case AgentState::Neutral:
                if (m_StateTicksRemaining > 0) {
                    return input;
                }
                if (absX > DISTANT_INTEREST_RANGE && !Chance(m_Config.DistantEngageProbability)) {
                    EnterState(AgentState::Neutral,
                               randomTicks(m_Config.NeutralMinTicks, m_Config.NeutralMaxTicks) +
                                   randomTicks(DISTANT_OBSERVE_EXTRA_MIN_TICKS,
                                               DISTANT_OBSERVE_EXTRA_MAX_TICKS));
                    return input;
                }
                if (absX > PREFERRED_MAX_RANGE) {
                    if (tryAerialDash(1.0f)) {
                        continue;
                    }
                    beginApproach();
                    return input;
                }
                beginPressure();
                continue;

            case AgentState::Approaching: {
                if (absX <= PREFERRED_MAX_RANGE) {
                    beginPressure();
                    continue;
                }
                if (m_StateTicksRemaining == 0 && canAct) {
                    if (tryAerialDash(1.0f)) {
                        continue;
                    }

                    float pursuitChance = m_Config.PursuitContinueProbability;
                    if (absX > FAR_PURSUIT_RANGE) {
                        pursuitChance *= FAR_PURSUIT_MULTIPLIER;
                    }
                    if (!Chance(pursuitChance)) {
                        beginNeutral();
                        return input;
                    }

                    planApproach();
                    m_StateTicksRemaining =
                        randomTicks(APPROACH_REPLAN_MIN_TICKS, APPROACH_REPLAN_MAX_TICKS);
                    useApproachMobility();
                }

                const float goalDistance = m_ApproachGoalX - observation.SelfPosition.x;
                const bool reachedGoal = std::abs(goalDistance) <= APPROACH_GOAL_DEAD_ZONE;
                const bool passedGoal =
                    !reachedGoal &&
                    SideFromDistance(goalDistance, m_MoveDirection) != m_MoveDirection;
                if (reachedGoal || passedGoal) {
                    beginNeutral();
                    return input;
                }

                Move(m_MoveDirection, input);
                return input;
            }

            case AgentState::Pressuring: {
                if (absX > REENGAGE_RANGE) {
                    if (Chance(m_Config.PursuitContinueProbability)) {
                        beginApproach();
                    } else {
                        beginNeutral();
                    }
                    return input;
                }
                if (m_StateTicksRemaining > 0 || !canAct) {
                    if (m_PressureMoveTicksRemaining > 0) {
                        Move(m_MoveDirection, input);
                    }
                    return input;
                }

                if (tryDashAttack()) {
                    continue;
                }

                const bool attackReady = m_AttackCooldownTicks == 0 && absX <= ATTACK_RANGE &&
                                         absY <= ATTACK_VERTICAL_RANGE;
                if (attackReady) {
                    const bool forceAttack = m_PressureSkips >= MAX_PRESSURE_SKIPS;
                    const float attackChance =
                        std::min(0.95f,
                                 m_Config.AttackProbability +
                                     ATTACK_PROBABILITY_STEP * static_cast<float>(m_PressureSkips));
                    if (forceAttack || Chance(attackChance)) {
                        m_PressureSkips = 0;
                        beginAttack();
                        continue;
                    }

                    ++m_PressureSkips;
                    if (tryAerialDash(ATTACK_READY_AIR_DASH_MULTIPLIER)) {
                        continue;
                    }
                    if (tryPressureJump(1.0f)) {
                        beginPressure();
                        return input;
                    }
                    if (Chance(m_Config.RetreatFeintProbability)) {
                        beginRetreat();
                        return input;
                    }
                } else {
                    if (tryAerialDash(1.0f)) {
                        continue;
                    }
                    if (tryPressureJump(1.0f)) {
                        beginPressure();
                        return input;
                    }
                    if (absX < PREFERRED_MIN_RANGE &&
                        Chance(RETREAT_WHEN_ATTACK_UNAVAILABLE_PROBABILITY)) {
                        beginRetreat();
                        return input;
                    }
                }

                beginPressure();
                if (m_PressureMoveTicksRemaining > 0) {
                    Move(m_MoveDirection, input);
                }
                return input;
            }

            case AgentState::AttackWindup:
                if (m_AttackFacingTicksRemaining > 0) {
                    Move(m_AttackDirection, input);
                    --m_AttackFacingTicksRemaining;
                }
                if (m_StateTicksRemaining > 0 || !canAct) {
                    return input;
                }
                Move(m_AttackDirection, input);
                input.AttackPressed = true;
                input.AttackHeld = true;
                m_AttackFacingTicksRemaining = 0;
                m_AttackCooldownTicks = ATTACK_COOLDOWN_TICKS;
                EnterState(AgentState::AttackRecovery, m_Config.AttackRecoveryTicks);
                return input;

            case AgentState::AttackRecovery:
                if (m_StateTicksRemaining > 0) {
                    return input;
                }
                if (Chance(POST_ATTACK_OBSERVE_PROBABILITY)) {
                    beginNeutral();
                    return input;
                }
                if (absX > PREFERRED_MAX_RANGE) {
                    beginApproach();
                    return input;
                }
                if ((absX < PREFERRED_MIN_RANGE &&
                     Chance(CLOSE_RETREAT_AFTER_ATTACK_PROBABILITY)) ||
                    (absX >= PREFERRED_MIN_RANGE && Chance(RETREAT_AFTER_ATTACK_PROBABILITY))) {
                    beginRetreat();
                    return input;
                }
                beginPressure();
                continue;

            case AgentState::Evading:
                if (m_StateTicksRemaining == 0 || absX >= RETREAT_STOP_RANGE) {
                    beginNeutral();
                    continue;
                }
                Move(m_MoveDirection, input);
                return input;

            case AgentState::Retreating:
                if (m_StateTicksRemaining == 0 || absX >= RETREAT_STOP_RANGE) {
                    beginNeutral();
                    continue;
                }
                Move(m_MoveDirection, input);
                return input;

            case AgentState::PerformingCombo:
                TickCombo(input);
                return input;
        }
    }

    beginNeutral();
    return input;
}

}  // namespace sop
