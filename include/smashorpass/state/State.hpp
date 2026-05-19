#pragma once

#include <string_view>
#include <type_traits>

#include "smashorpass/core/Event.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

class State {
   public:
    virtual ~State() = default;

    virtual sop_util::Result<void> Initialize(AppCtx& ctx) = 0;

    [[nodiscard]] virtual std::string_view DebugName() const = 0;

    virtual sop_util::Result<EventFlow> OnEvent(AppCtx&, const Event&) {
        return sop_util::Ok(EventFlow::Passed);
    }
    virtual sop_util::Result<void> OnUpdate(AppCtx&) {
        return sop_util::Ok();
    }
    virtual sop_util::Result<void> OnRender(AppCtx&) {
        return sop_util::Ok();
    }
};

template <typename TState>
concept IsState = std::is_base_of_v<State, TState>;
}  // namespace sop
