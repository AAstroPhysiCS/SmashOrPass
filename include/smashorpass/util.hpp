#pragma once

#include <expected>
#include <string>
#include <type_traits>
#include <utility>

namespace sop_util {

// ---- RESULT ------------------------------------------------

// Usage:
// Result<T> is std::expected<T, std::string> by default.
// Return Ok(value) on success, Err(error) on failure.
// Use Ok() for successful Result<void>.
//
// TRY(name, expr) unwraps a Result or returns its error.
// TRY_VOID(expr) does the same for Result<void>.

// Result, Ok() and Err()
template <class T, class E = std::string>
using Result = std::expected<T, E>;

template <class E>
constexpr auto Err(E&& error) {
    return std::unexpected<std::decay_t<E>>(std::forward<E>(error));
}

template <class T>
constexpr auto Ok(T&& value) {
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

// TRY and TRY_VOID macro for ergonomics
#define TRY_CONCAT_IMPL(a, b) a##b
#define TRY_CONCAT(a, b) TRY_CONCAT_IMPL(a, b)

#define TRY_IMPL(result_var, value_var, expr)                             \
    auto result_var = (expr);                                             \
    if (!result_var.has_value()) {                                        \
        return sop_util::Err(std::move(result_var).error());              \
    }                                                                     \
    auto value_var = *std::move(result_var)

#define TRY(value_var, expr)                                              \
    TRY_IMPL(TRY_CONCAT(_try_result_, __COUNTER__), value_var, expr)

#define TRY_VOID_IMPL(result_var, expr)                                   \
    do {                                                                  \
        auto result_var = (expr);                                         \
        if (!result_var.has_value()) {                                    \
            return sop_util::Err(std::move(result_var).error());          \
        }                                                                 \
    } while (false)

#define TRY_VOID(expr)                                                    \
    TRY_VOID_IMPL(TRY_CONCAT(_try_result_, __COUNTER__), expr)

}  // namespace sop_util
