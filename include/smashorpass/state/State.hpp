#pragma once

#include <type_traits>

#include "smashorpass/core/Event.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

using namespace sop_util;

class State {
   public:
    virtual ~State() = default;

    virtual Result<EventFlow> OnEvent(AppCtx&, const Event&) {
        return Ok(EventFlow::Passed);
    }
    virtual Result<void> OnUpdate(AppCtx&) {
        return Ok();
    }
    virtual Result<void> OnRender(AppCtx&) {
        return Ok();
    }
};

template <typename TState>
concept IsState = std::is_base_of_v<State, TState>;
}  // namespace sop
