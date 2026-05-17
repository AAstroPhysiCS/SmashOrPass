#include <print>
#include "smashorpass/app/Application.hpp"

int main() {
    sop::Application application;
    auto result = application.Run();
    if (!result) {
        std::println(stderr, "Application error: {}", result.error());
        return 1;
    }
    return 0;
}
