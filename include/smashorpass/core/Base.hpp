#pragma once

#include <SDL.h>

#include <cstdlib>
#include <iostream>

#if defined(_MSC_VER)
    #define SOP_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #define SOP_DEBUG_BREAK() __builtin_trap()
#else
    #include <cstdlib>
    #define SOP_DEBUG_BREAK() std::abort()
#endif

#ifndef NDEBUG
    #define SOP_ENABLE_ASSERTS 1
#else
    #define SOP_ENABLE_ASSERTS 0
#endif

namespace sop::detail {
    inline void report_assert(const char *expr, const char *message, const char *file, int line) {
        std::cerr << "[ASSERT] Expression: " << expr << '\n'
                  << "[ASSERT] Message   : " << (message ? message : "") << '\n'
                  << "[ASSERT] File      : " << file << ':' << line << '\n';
    }

    inline void report_sdl_assert(const char *expr, const char *message, const char *file, int line) {
        std::cerr << "[SDL ASSERT] Expression: " << expr << '\n'
                  << "[SDL ASSERT] Message   : " << (message ? message : "") << '\n'
                  << "[SDL ASSERT] SDL Error : " << SDL_GetError() << '\n'
                  << "[SDL ASSERT] File      : " << file << ':' << line << '\n';
    }
}

#if SOP_ENABLE_ASSERTS

#define SOP_ASSERT(expr, message)                                                                  \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            ::sop::detail::report_assert(#expr, message, __FILE__, __LINE__);                      \
            SOP_DEBUG_BREAK();                                                                     \
            std::abort();                                                                          \
        }                                                                                          \
    } while (false)

#define SOP_SDL_ASSERT(expr, message)                                                              \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            ::sop::detail::report_sdl_assert(#expr, message, __FILE__, __LINE__);                  \
            SOP_DEBUG_BREAK();                                                                     \
            std::abort();                                                                          \
        }                                                                                          \
    } while (false)

#else

#define SOP_ASSERT(expr, message) ((void)0)
#define SOP_SDL_ASSERT(expr, message) ((void)0)

#endif

#define SOP_VERIFY(expr, message)                                                                  \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            ::sop::detail::report_assert(#expr, message, __FILE__, __LINE__);                      \
            std::abort();                                                                          \
        }                                                                                          \
    } while (false)                                                                                \