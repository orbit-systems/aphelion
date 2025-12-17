#ifndef ORBIT_PORTABILITY_H

#if defined(_WIN32)
    #define OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
#elif defined(__linux__)
    #define OS_LINUX
    #define OS_UNIX
    #define _GNU_SOURCE
#elif defined(__APPLE__) && defined(__MACH__)
    #define OS_MACOS
    #define OS_UNIX
    #define _DARWIN_C_SOURCE
#else
    #error currently unknown platform! contact sandwich
#endif

#ifndef nullptr
    #include <stddef.h>
    #define nullptr NULL
#endif

#if defined(__GNUC__) || defined(__clang__)
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

#if defined(__GNUC__) || defined(__clang__)
    #define FALLTHROUGH __attribute__((fallthrough))
    #define UNUSED __attribute__((unused))
    #define NORETURN __attribute__((noreturn))
#else
    #define UNUSED /* unused */
    #define NORETURN /* noreturn */
    #define FALLTHROUGH /* fallthrough */
#endif

#endif // ORBIT_PORTABILITY_H
