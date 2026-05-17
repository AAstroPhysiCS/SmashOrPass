#include <print>

#include "smashorpass/core/Application.hpp"

int main() {
    sop::Application application;
    auto result = application.Initialize();
    if (!result) {
        std::println(stderr, "Application initialization error: {}", result.error());
        return 1;
    }

    result = application.Run();
    if (!result) {
        std::println(stderr, "Application error: {}", result.error());
        return 1;
    }
    return 0;
}
