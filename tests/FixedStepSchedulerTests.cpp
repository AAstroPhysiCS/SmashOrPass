#include "smashorpass/core/FixedStepScheduler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>

TEST_CASE("120 hz gameplay scheduler advances twice for a 60 hz frame")
{
    sop::FixedStepScheduler scheduler(120);

    const uint32_t ticksDue = scheduler.Advance(std::chrono::duration<double>(1.0 / 60.0));

    REQUIRE(ticksDue == 2);
    REQUIRE(scheduler.GetTotalTicks() == 2);
}

TEST_CASE("30 hz animation scheduler accumulates independently across frames")
{
    sop::FixedStepScheduler gameplayScheduler(120);
    sop::FixedStepScheduler animationScheduler(30);

    const auto frameDelta = std::chrono::duration<double>(1.0 / 60.0);

    REQUIRE(gameplayScheduler.Advance(frameDelta) == 2);
    REQUIRE(animationScheduler.Advance(frameDelta) == 0);

    REQUIRE(gameplayScheduler.Advance(frameDelta) == 2);
    REQUIRE(animationScheduler.Advance(frameDelta) == 1);

    REQUIRE(gameplayScheduler.GetTotalTicks() == 4);
    REQUIRE(animationScheduler.GetTotalTicks() == 1);
}

TEST_CASE("scheduler produces the same total ticks for irregular frame pacing")
{
    sop::FixedStepScheduler steadyScheduler(120);
    sop::FixedStepScheduler unevenScheduler(120);

    REQUIRE(steadyScheduler.Advance(std::chrono::milliseconds(20)) == 2);

    REQUIRE(unevenScheduler.Advance(std::chrono::milliseconds(5)) == 0);
    REQUIRE(unevenScheduler.Advance(std::chrono::milliseconds(7)) == 1);
    REQUIRE(unevenScheduler.Advance(std::chrono::milliseconds(8)) == 1);

    REQUIRE(steadyScheduler.GetTotalTicks() == unevenScheduler.GetTotalTicks());
}

TEST_CASE("scheduler clamps large frame spikes before generating ticks")
{
    sop::FixedStepScheduler scheduler(120);

    const uint32_t ticksDue = scheduler.Advance(std::chrono::seconds(1));

    REQUIRE(ticksDue == 30);
    REQUIRE(scheduler.GetTotalTicks() == 30);
}
