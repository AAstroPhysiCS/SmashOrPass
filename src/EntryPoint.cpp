#include "smashorpass/core/Application.hpp"

int main() {
    sop::Application application;
    auto result = application.Run();
    if (!result) {
        spdlog::error("Application failed with error: {}", result.error());
        return 1;
    }
    return 0;
}
