#include "smashorpass/state/StateManager.hpp"

#include "smashorpass/app/AppCtx.hpp"

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
    if (ctx.DebugOverlayVisible) {
        for (auto it = m_Overlays.rbegin(); it != m_Overlays.rend(); ++it) {
            auto result = (*it)->OnEvent(ctx, event);
            if (!result) {
                return result;
            }
            if (*result == EventFlow::Consumed) {
                return Ok(EventFlow::Consumed);
            }
        }
    }

    State* state = TopState();
    if (state != nullptr) {
        auto result = state->OnEvent(ctx, event);
        if (!result) {
            return result;
        }
        if (*result == EventFlow::Consumed) {
            return Ok(EventFlow::Consumed);
        }
    }

    return Ok(EventFlow::Passed);
}

Result<void> StateManager::Update(AppCtx& ctx) {
    State* state = TopState();
    if (state != nullptr) {
        auto result = state->OnUpdate(ctx);
        if (!result) {
            return result;
        }
    }

    if (ctx.DebugOverlayVisible) {
        for (const auto& overlay : m_Overlays) {
            auto result = overlay->OnUpdate(ctx);
            if (!result) {
                return result;
            }
        }
    }

    return Ok();
}

Result<void> StateManager::Render(AppCtx& ctx) {
    for (const auto& state : m_States) {
        auto result = state->OnRender(ctx);
        if (!result) {
            return result;
        }
    }

    if (ctx.DebugOverlayVisible) {
        for (const auto& overlay : m_Overlays) {
            auto result = overlay->OnRender(ctx);
            if (!result) {
                return result;
            }
        }
    }

    return Ok();
}

}  // namespace sop
