#include "smashorpass/state/StateManager.hpp"

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

StateManager::~StateManager() = default;

void StateManager::PopState() {
    if (!m_States.empty()) {
        m_States.pop_back();
    }
}

void StateManager::ClearStates() {
    m_States.clear();
}

void StateManager::ClearOverlays() {
    m_Overlays.clear();
}

bool StateManager::HasStates() const {
    return !m_States.empty();
}

State* StateManager::TopState() {
    if (m_States.empty()) {
        return nullptr;
    }
    return m_States.back().get();
}

const State* StateManager::TopState() const {
    if (m_States.empty()) {
        return nullptr;
    }
    return m_States.back().get();
}

Result<EventFlow> StateManager::DispatchEvent(AppCtx& ctx, const Event& event) {
    // TODO: Rethink this should we ever introduce a non debug overlay
    if (ctx.debugOverlayVisible) {
        for (auto it = m_Overlays.rbegin(); it != m_Overlays.rend(); ++it) {
            TRY(eventFlow, (*it)->OnEvent(ctx, event));
            if (eventFlow == EventFlow::Consumed) {
                return Ok(EventFlow::Consumed);
            }
        }
    }

    State* state = TopState();
    if (state != nullptr) {
        TRY(eventFlow, state->OnEvent(ctx, event));
        if (eventFlow == EventFlow::Consumed) {
            return Ok(EventFlow::Consumed);
        }
    }

    return Ok(EventFlow::Passed);
}

Result<void> StateManager::Update(AppCtx& ctx) {
    State* state = TopState();
    if (state != nullptr) {
        TRY_VOID(state->OnUpdate(ctx));
    }

    if (ctx.debugOverlayVisible) {
        for (const auto& overlay : m_Overlays) {
            TRY_VOID(overlay->OnUpdate(ctx));
        }
    }

    return Ok();
}

Result<void> StateManager::Render(AppCtx& ctx) {
    for (const auto& state : m_States) {
        TRY_VOID(state->OnRender(ctx));
    }

    if (ctx.debugOverlayVisible) {
        for (const auto& overlay : m_Overlays) {
            TRY_VOID(overlay->OnRender(ctx));
        }
    }

    return Ok();
}

}  // namespace sop
