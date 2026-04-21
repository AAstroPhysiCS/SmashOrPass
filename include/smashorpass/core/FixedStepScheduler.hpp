#pragma once

#include <chrono>
#include <cstdint>

namespace sop {

    class FixedStepScheduler final {
    public:
        using Duration = std::chrono::duration<long double>;

        explicit FixedStepScheduler(uint32_t ticksPerSecond, Duration maxFrameDelta = std::chrono::milliseconds(250));

        template <typename Rep, typename Period>
        uint32_t Advance(std::chrono::duration<Rep, Period> elapsed)
        {
            return Advance(std::chrono::duration_cast<Duration>(elapsed));
        }

        [[nodiscard]] uint32_t Advance(Duration elapsed);
        void Reset();

        [[nodiscard]] uint32_t GetTicksPerSecond() const { return m_TicksPerSecond; }
        [[nodiscard]] Duration GetMaxFrameDelta() const { return m_MaxFrameDelta; }
        [[nodiscard]] uint64_t GetTotalTicks() const { return m_TotalTicks; }

    private:
        uint32_t m_TicksPerSecond = 0;
        Duration m_MaxFrameDelta = Duration::zero();
        long double m_AccumulatedTicks = 0.0L;
        uint64_t m_TotalTicks = 0;
    };
}  // namespace sop
