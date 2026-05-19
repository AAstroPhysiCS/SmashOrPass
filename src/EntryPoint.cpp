#include <iostream>

#include "smashorpass/core/Application.hpp"

int main() {
    sop::Application application;
    auto result = application.Initialize();
    if (!result) {
        std::cerr << "Application initialization error: " << result.error() << '\n';
        return 1;
    }

    result = application.Run();
    if (!result) {
        std::cerr << "Application error: " << result.error() << '\n';
        return 1;
    }
    return 0;
}
