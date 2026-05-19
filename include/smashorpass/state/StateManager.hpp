#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "smashorpass/state/State.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

class StateManager {
   public:
    StateManager() = default;
    ~StateManager();

    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) = delete;
    StateManager& operator=(StateManager&&) = delete;

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    sop_util::Result<std::reference_wrapper<TState>> PushState(AppCtx& ctx, TArgs&&... args) {
        auto state = std::make_unique<TState>(ctx, std::forward<TArgs>(args)...);
        TState& ref = *state;
        TRY_VOID(ref.Initialize(ctx));
        m_States.push_back(std::move(state));
        return sop_util::Ok(std::ref(ref));
    }

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    sop_util::Result<std::reference_wrapper<TState>> PushOverlay(AppCtx& ctx, TArgs&&... args) {
        auto state = std::make_unique<TState>(ctx, std::forward<TArgs>(args)...);
        TState& ref = *state;
        TRY_VOID(ref.Initialize(ctx));
        m_Overlays.push_back(std::move(state));
        return sop_util::Ok(std::ref(ref));
    }

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    sop_util::Result<std::reference_wrapper<TState>> ReplaceTopState(AppCtx& ctx, TArgs&&... args) {
        auto state = std::make_unique<TState>(ctx, std::forward<TArgs>(args)...);
        TState& ref = *state;
        TRY_VOID(ref.Initialize(ctx));
        if (m_States.empty()) {
            m_States.push_back(std::move(state));
        } else {
            m_States.back() = std::move(state);
        }
        return sop_util::Ok(std::ref(ref));
    }

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    sop_util::Result<std::reference_wrapper<TState>> ResetToState(AppCtx& ctx, TArgs&&... args) {
        auto state = std::make_unique<TState>(ctx, std::forward<TArgs>(args)...);
        TState& ref = *state;
        TRY_VOID(ref.Initialize(ctx));
        ClearStates();
        m_States.push_back(std::move(state));
        return sop_util::Ok(std::ref(ref));
    }

    void PopState();
    void ClearStates();
    void ClearOverlays();

    [[nodiscard]] bool HasStates() const;
    [[nodiscard]] State* TopState();
    [[nodiscard]] const State* TopState() const;

    template <typename TState>
        requires IsState<TState>
    [[nodiscard]] TState* TopStateAs() {
        return dynamic_cast<TState*>(TopState());
    }

    template <typename TState>
        requires IsState<TState>
    [[nodiscard]] const TState* TopStateAs() const {
        return dynamic_cast<const TState*>(TopState());
    }

    sop_util::Result<EventFlow> DispatchEvent(AppCtx& ctx, const Event& event);
    sop_util::Result<void> Update(AppCtx& ctx);
    sop_util::Result<void> Render(AppCtx& ctx);

   private:
    std::vector<std::unique_ptr<State>> m_Overlays;
    std::vector<std::unique_ptr<State>> m_States;
};
}  // namespace sop
