#pragma once

#include <expected>
#include <string>
#include <type_traits>
#include <utility>

namespace sop_util {

// ---- RESULT ------------------------------------------------

template <class T, class E = std::string>
using Result = std::expected<T, E>;

template <class E>
constexpr auto Err(E &&error) {
    return std::unexpected<std::decay_t<E>>(std::forward<E>(error));
}

template <class T>
constexpr auto Ok(T &&value) {
    return std::forward<T>(value);
}

struct OkVoid {
    template <class E = std::string>
    constexpr operator Result<void, E>() const {
        return {};
    }
};

constexpr OkVoid Ok() {
    return {};
}

} // namespace sop_util
