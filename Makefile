
LUNA_SRC_PATHS = \
	src/luna/*.c \
	src/common/*.c \

LUNA_SRC = $(wildcard $(LUNA_SRC_PATHS))
LUNA_OBJECTS = $(LUNA_SRC:src/%.c=build/%.o)

ALL_SRC_PATHS = \
	$(LUNA_SRC_PATHS)
ALL_SRC = $(wildcard $(ALL_SRC_PATHS))


CC = gcc
LD = gcc

INCLUDEPATHS = -Iinclude/
ASANFLAGS = -fsanitize=undefined -fsanitize=address
CFLAGS = -std=gnu23 -fwrapv -fno-strict-aliasing
WARNINGS = -Wall -Wimplicit-fallthrough -Wno-override-init -Wno-enum-compare -Wno-unused -Wno-enum-conversion -Wno-discarded-qualifiers -Wno-strict-aliasing
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

FILE_NUM = 0
build/%.o: src/%.c
	$(eval FILE_NUM=$(shell echo $$(($(FILE_NUM)+1))))
	$(shell echo 1>&2 -e "Compiling \e[1m$<\e[0m")
	
	@$(CC) -c -o $@ $< -MD $(INCLUDEPATHS) $(ALLFLAGS) $(OPT)

.PHONY: tidy
tidy:
	clang-tidy -checks='-*,readability-braces-around-statements' -fix-errors $(ALL_SRC)
	

.PHONY: luna
luna: bin/luna
bin/luna: $(LUNA_OBJECTS)
	@$(LD) $(LDFLAGS) $(LUNA_OBJECTS) -o bin/luna

.PHONY: clean
clean:
	@rm -rf build/
	@rm -rf bin/
	@mkdir build/
	@mkdir bin/
	@mkdir -p $(dir $(LUNA_OBJECTS))

-include $(LUNA_OBJECTS:.o=.d)

# generate compile commands with bear if u got it!!! 
# very good highly recommended ʕ·ᴥ·ʔ
.PHONY: bear-gen-cc
bear-gen-cc: clean
	bear -- $(MAKE) luna
