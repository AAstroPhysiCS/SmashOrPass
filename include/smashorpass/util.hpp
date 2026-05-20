#pragma once

#include <SDL3/SDL_error.h>

#include <expected>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace sop {

template <class E>
constexpr auto Err(E&& error) {
    return std::unexpected<std::decay_t<E>>(std::forward<E>(error));
}

template <class T>
constexpr auto Ok(T&& value) {
    return std::forward<T>(value);
}

// ---- RESULT ------------------------------------------------

// Usage:
// Result<T> is std::expected<T, std::string> by default.
// Return Ok(value) on success, Err(error) on failure.
// Use Ok() for successful Result<void>.
//
// TRY(name, expr) unwraps a Result or returns its error.
// TRY_VOID(expr) does the same for Result<void>.
//
// Result is always [[nodiscard]].

// Result, Ok() and Err()

template <class T, class E = std::string>
struct [[nodiscard("Result must be checked")]] Result : std::expected<T, E> {
    using Base = std::expected<T, E>;

    using Base::Base;
    using Base::operator bool;
    using Base::operator=;

    template <class U>
    Result& operator&=(const Result<U, E>& other) {
        // Keep existing error if already failed
        if (!*this)
            return *this;
        // Otherwise copy/move failure from other
        if (!other)
            *this = Err(other.error());
        return *this;
    }
};

struct OkVoid {
    template <class E = std::string>
    constexpr operator Result<void, E>() const {
        return {};
    }
};

constexpr OkVoid Ok() {
    return {};
}

inline std::string SdlError(std::string_view operation) {
    return std::string(operation) + " failed: " + SDL_GetError();
}

inline Result<void> SdlResult(bool ok, std::string_view operation) {
    if (!ok) {
        return Err(SdlError(operation));
    }
    return Ok();
}

// TRY and TRY_VOID macro for ergonomics
// Note: __COUNTER__ is a compiler extension, but it is supported everywhere we tested.
#if defined(__clang__)
#if __has_warning("-Wc2y-extensions")
#define SOP_UTIL_CLANG_SUPPRESS_COUNTER_WARNING \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wc2y-extensions\"")
#define SOP_UTIL_CLANG_RESTORE_COUNTER_WARNING _Pragma("clang diagnostic pop")
#else
#define SOP_UTIL_CLANG_SUPPRESS_COUNTER_WARNING
#define SOP_UTIL_CLANG_RESTORE_COUNTER_WARNING
#endif
#else
#define SOP_UTIL_CLANG_SUPPRESS_COUNTER_WARNING
#define SOP_UTIL_CLANG_RESTORE_COUNTER_WARNING
#endif

#define SOP_UTIL_TRY_CONCAT_IMPL(a, b) a##b
#define SOP_UTIL_TRY_CONCAT(a, b) SOP_UTIL_TRY_CONCAT_IMPL(a, b)

#define SOP_UTIL_TRY_IMPL(result_var, value_var, expr)  \
    auto result_var = (expr);                           \
    if (!result_var.has_value()) {                      \
        return sop::Err(std::move(result_var).error()); \
    }                                                   \
    auto value_var = *std::move(result_var)

#define SOP_UTIL_TRY_AND_IMPL(accumulator, result_var, expr) \
    do {                                                     \
        auto result_var = (expr);                            \
        (accumulator) &= result_var;                         \
    } while (false)

#define TRY(value_var, expr)                                                            \
    SOP_UTIL_CLANG_SUPPRESS_COUNTER_WARNING                                             \
    SOP_UTIL_TRY_IMPL(SOP_UTIL_TRY_CONCAT(_try_result_, __COUNTER__), value_var, expr); \
    SOP_UTIL_CLANG_RESTORE_COUNTER_WARNING

#define SOP_UTIL_TRY_VOID_IMPL(result_var, expr)            \
    do {                                                    \
        auto result_var = (expr);                           \
        if (!result_var.has_value()) {                      \
            return sop::Err(std::move(result_var).error()); \
        }                                                   \
    } while (false)

#define TRY_VOID(expr)                                                            \
    SOP_UTIL_CLANG_SUPPRESS_COUNTER_WARNING                                       \
    SOP_UTIL_TRY_VOID_IMPL(SOP_UTIL_TRY_CONCAT(_try_result_, __COUNTER__), expr); \
    SOP_UTIL_CLANG_RESTORE_COUNTER_WARNING

#define TRY_AND_VOID(accumulator, expr)                                                       \
    SOP_UTIL_CLANG_SUPPRESS_COUNTER_WARNING                                                   \
    SOP_UTIL_TRY_AND_IMPL(accumulator, SOP_UTIL_TRY_CONCAT(_try_result_, __COUNTER__), expr); \
    SOP_UTIL_CLANG_RESTORE_COUNTER_WARNING

}  // namespace sop
