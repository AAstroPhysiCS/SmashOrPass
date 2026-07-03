#include "smashorpass/state/states/match_results/MatchResultsState.hpp"

#include <utility>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

MatchResultsState::MatchResultsState(AppCtx& ctx,
                                     MatchConfig matchConfig,
                                     MatchResults results)
    : m_Screen(ctx, std::move(matchConfig)), m_Results(std::move(results)) {
    UIBuilder builder(m_Screen);
    m_Screen.Build(builder);
}

Result<void> MatchResultsState::Initialize(AppCtx&) {
    m_Screen.SetResults(m_Results);
    return Ok();
}

Result<EventFlow> MatchResultsState::OnEvent(AppCtx& ctx, const Event& event) {
    return Ok(m_Screen.OnEvent(ctx, event));
}

Result<void> MatchResultsState::OnUpdate(AppCtx& ctx) {
    m_Screen.OnUpdate(ctx);
    return Ok();
}

Result<void> MatchResultsState::OnRender(AppCtx& ctx) {
    return m_Screen.OnRender(ctx);
}

}  // namespace sop
