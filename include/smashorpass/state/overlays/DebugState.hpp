#pragma once

#include "smashorpass/state/State.hpp"

namespace sop {

class DebugState : public State {
   public:
    explicit DebugState(AppCtx& ctx);
    ~DebugState() override;

    [[nodiscard]] std::string_view DebugName() const final {
        return "Debug";
    }

    void BeginFrame();
    void Draw(AppCtx& ctx);
    void EndFrame(AppCtx& ctx);

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnRender(AppCtx& ctx) final;
};

}  // namespace sop
