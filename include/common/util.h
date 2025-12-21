#ifndef ORBIT_UTIL_H
#define ORBIT_UTIL_H

#include <stdio.h>

#if defined(_WIN32)
    #define OS_WINDOWS
#elif defined(__linux__)
    #define OS_LINUX
#else
    #error currently unknown platform! contact sandwich
#endif

#define TODO(msg, ...) do {\
    printf("\x1b[36m\x1b[1mTODO\x1b[0m in %s() at %s:%d -> " msg "\n", (__func__), (__FILE__), (__LINE__) __VA_OPT__(,) __VA_ARGS__); \
    exit(EXIT_FAILURE);} while (0)

#define CRASH(msg, ...) do {\
    printf("\x1b[31m\x1b[1mCRASH\x1b[0m in %s() at %s:%d -> " msg "\n", (__func__), (__FILE__), (__LINE__) __VA_OPT__(,) __VA_ARGS__); \
    exit(EXIT_FAILURE);} while (0)

#define UNREACHABLE do { \
    printf("\x1b[31m\x1b[1mUNREACHABLE\x1b[0m in %s() at %s:%d\n", (__func__) ,(__FILE__), (__LINE__)); \
    exit(EXIT_FAILURE);} while (0)

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define for_n_eq(iterator, start, end) for (isize iterator = (start); iterator <= (end); ++iterator)
#define for_n(iterator, start, end) for (isize iterator = (start); iterator < (end); ++iterator)
#define for_n_reverse(iterator, start, end) for (isize iterator = (start) - 1; iterator >= (end); --iterator)

#ifdef __GNUC__
    #define if_likely(x)   if (__builtin_expect((bool)(x), true))
    #define if_unlikely(x) if (__builtin_expect((bool)(x), false))
    #define while_likely(x)   while (__builtin_expect((bool)(x), true))
    #define while_unlikely(x) while (__builtin_expect((bool)(x), false))
#else
    #define if_likely(x)   if (x)
    #define if_unlikely(x) if (x)
    #define while_likely(x)   while (x)
    #define while_unlikely(x) while (x)
#endif

#define is_pow_2(i) ((i) != 0 && ((i) & ((i)-1)) == 0)

#endif // ORBIT_UTIL_H
