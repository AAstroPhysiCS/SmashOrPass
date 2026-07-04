#pragma once

#include <string_view>

#include "smashorpass/state/State.hpp"
#include "smashorpass/state/states/in_game/MatchConfig.hpp"
#include "smashorpass/state/states/match_results/ui/MatchResultsScreen.hpp"

namespace sop {

class MatchResultsState final : public State {
   public:
    explicit MatchResultsState(AppCtx& ctx,
                               MatchConfig matchConfig = {},
                               MatchResults results = {});
    ~MatchResultsState() override = default;

    Result<void> Initialize(AppCtx& ctx) final;

    [[nodiscard]] std::string_view DebugName() const final {
        return "MatchResults";
    }

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

   private:
    MatchResultsScreen m_Screen;
    MatchResults m_Results{};
};

}  // namespace sop
