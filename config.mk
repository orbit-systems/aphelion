# Customize below to fit your system

CC =
LD =

CFLAGS = -std=c99 -fwrapv -fno-strict-aliasing
LDFLAGS =
TIDYFLAGS = -checks='-*,readability-braces-around-statements' -fix-errors

INCLUDEPATHS = -Iinclude/ -Icommon/include/

# Linux
LDFLAGS += -rdynamic

## Macos
CC = clang
LD = clang

SDK_PATH = $(shell xcrun --show-sdk-path)

CFLAGS += -arch arm64 -isysroot $(SDK_PATH)
LDFLAGS += -arch arm64 -isysroot $(SDK_PATH)
TIDYFLAGS += --extra-arg=-isysroot$(SDK_PATH)
## Windows

## Release
#CFLAGS += -g0 -O3

## Dev
CFLAGS += -g3 -O0

WARNINGS = -Wall -Wimplicit-fallthrough -Wno-override-init -Wno-enum-compare \
           -Wno-unused -Wno-enum-conversion -Wno-discarded-qualifiers        \
           -Wno-ignored-qualifiers -Wno-strict-aliasing                      \
           -Wno-unknown-warning-option
# Note: We mix clang and gcc flags here because its simpler and they both
# ignore eachother when they dont recognise one. Maybe you could split this
# but for now its not such an issue.

# To enable address sanitation, call make with ASAN_ENABLE=1, or uncomment
# the line below to enable it.
# ASAN_ENABLE = 1
ASANFLAGS = -fsanitize=undefined -fsanitize=address
