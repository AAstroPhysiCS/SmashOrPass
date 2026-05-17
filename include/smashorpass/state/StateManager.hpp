#pragma once

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
    TState& PushState(AppCtx& ctx, TArgs&&... args) {
        auto state = std::make_unique<TState>(ctx, std::forward<TArgs>(args)...);
        TState& ref = *state;
        m_States.push_back(std::move(state));
        return ref;
    }

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    TState& PushOverlay(AppCtx& ctx, TArgs&&... args) {
        auto state = std::make_unique<TState>(ctx, std::forward<TArgs>(args)...);
        TState& ref = *state;
        m_Overlays.push_back(std::move(state));
        return ref;
    }

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    TState& ReplaceTopState(AppCtx& ctx, TArgs&&... args) {
        PopState();
        return PushState<TState>(ctx, std::forward<TArgs>(args)...);
    }

    template <typename TState, typename... TArgs>
        requires IsState<TState>
    TState& ResetToState(AppCtx& ctx, TArgs&&... args) {
        ClearStates();
        return PushState<TState>(ctx, std::forward<TArgs>(args)...);
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

    Result<EventFlow> DispatchEvent(AppCtx& ctx, const Event& event);
    Result<void> Update(AppCtx& ctx);
    Result<void> Render(AppCtx& ctx);

   private:
    std::vector<std::unique_ptr<State>> m_Overlays;
    std::vector<std::unique_ptr<State>> m_States;
};
}  // namespace sop
