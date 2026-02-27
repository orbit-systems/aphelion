BUILD_DIR = build

# libcommon config
export COMMON_OUT_DIR=../$(BUILD_DIR)

LUNA_SRC_PATHS = \
	src/luna/*.c \

LUNA_SRC = $(wildcard $(LUNA_SRC_PATHS))
LUNA_OBJECTS = $(LUNA_SRC:src/%.c=build/%.o)

CC ?= gcc
LD = $(CC)

INCLUDEPATHS = -Iinclude/ -Icommon/include/
ASANFLAGS = -fsanitize=undefined -fsanitize=address
CFLAGS = -std=gnu2x -fwrapv -fno-strict-aliasing
WARNINGS = \
	-Wall -Wimplicit-fallthrough -Wmaybe-uninitialized \
	-Wno-enum-compare -Wno-unused -Wno-enum-conversion -Wno-discarded-qualifiers
ALLFLAGS = $(CFLAGS) $(WARNINGS)
OPT = -g3 -O0

LDFLAGS =

ifneq ($(OS),Windows_NT)
	CFLAGS += -rdynamic
endif

ifdef ASAN_ENABLE
	CFLAGS += $(ASANFLAGS)
	LDFLAGS += $(ASANFLAGS)
endif

#include configuration file, if present
-include config.mk

.PHONY: all
all: luna

bin/libcommon.a:
	$(MAKE) -C common
	cp $(BUILD_DIR)/libcommon.a bin/libcommon.a

build/%.o: src/%.c
	$(shell echo 1>&2 -e "Compiling $<")
	@$(CC) -c -o $@ $< -MD $(INCLUDEPATHS) $(ALLFLAGS) $(OPT)

.PHONY: luna
luna: bin/luna
bin/luna:  $(LUNA_OBJECTS) bin/libcommon.a
	@$(LD) $(LDFLAGS) $(LUNA_OBJECTS) -o bin/luna -lm -lc -Lbin -lcommon

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)/
	@rm -rf bin/
	@mkdir $(BUILD_DIR)/
	@mkdir bin/
	@mkdir -p $(dir $(LUNA_OBJECTS))

# generate compile commands with bear if u got it!!!
# very good highly recommended ʕ·ᴥ·ʔ
.PHONY: bear-gen-cc
bear-gen-cc: clean
	bear -- $(MAKE) all

-include $(LUNA_OBJECTS:.o=.d)
