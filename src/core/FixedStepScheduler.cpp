#include "smashorpass/core/FixedStepScheduler.hpp"

#include <algorithm>
#include <cmath>

#include "smashorpass/core/Base.hpp"

namespace sop {

FixedStepScheduler::FixedStepScheduler(uint32_t ticksPerSecond, Duration maxFrameDelta)
    : m_TicksPerSecond(ticksPerSecond), m_MaxFrameDelta(maxFrameDelta) {
    SOP_ASSERT(m_TicksPerSecond > 0, "FixedStepScheduler requires a non-zero tick rate");
    SOP_ASSERT(m_MaxFrameDelta > Duration::zero(),
               "FixedStepScheduler requires a positive max frame delta");
}

uint32_t FixedStepScheduler::Advance(Duration elapsed) {
    const Duration clampedElapsed = std::clamp(elapsed, Duration::zero(), m_MaxFrameDelta);
    constexpr long double kRoundingEpsilon = 1.0e-12L;

    m_AccumulatedTicks += clampedElapsed.count() * static_cast<long double>(m_TicksPerSecond);

    const uint32_t ticksDue =
        static_cast<uint32_t>(std::floor(m_AccumulatedTicks + kRoundingEpsilon));
    m_AccumulatedTicks -= static_cast<long double>(ticksDue);
    m_TotalTicks += static_cast<uint64_t>(ticksDue);

    return ticksDue;
}

void FixedStepScheduler::Reset() {
    m_AccumulatedTicks = 0.0L;
    m_TotalTicks = 0;
}
}  // namespace sop
