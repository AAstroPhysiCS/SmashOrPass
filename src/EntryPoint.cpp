#include <print>
#include "smashorpass/app/Application.hpp"
#include "smashorpass/state/DebugState.hpp"

int main() {
    sop::Application application;
    application.PushOverlay<sop::DebugState>();
    auto result = application.Run();
    if (!result) {
        std::println(stderr, "Application error: {}", result.error());
        return 1;
    }
    return 0;
}
