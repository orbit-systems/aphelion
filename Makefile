include config.mk

COMMON_OUT_DIR = $(abspath build/common)

LUNA_SRC_PATHS = \
	src/luna/*.c

LUNA_SRC = $(wildcard $(LUNA_SRC_PATHS))
LUNA_OBJECTS = $(LUNA_SRC:src/%.c=build/%.o)

COMMON_LIB = $(COMMON_OUT_DIR)/libcommon.a

ALL_SRC_PATHS = \
	$(LUNA_SRC_PATHS)
ALL_SRC = $(wildcard $(ALL_SRC_PATHS))

ALLFLAGS = $(CFLAGS) $(WARNINGS)

ifdef ASAN_ENABLE
	CFLAGS += $(ASANFLAGS)
	LDFLAGS += $(ASANFLAGS)
endif

FILE_NUM = 0
build/%.o: src/%.c
	$(eval FILE_NUM=$(shell echo $$(($(FILE_NUM)+1))))
	$(shell echo 1>&2 -e "Compiling \e[1m$<\e[0m")
	
	@$(CC) -c -o $@ $< -MD $(INCLUDEPATHS) $(ALLFLAGS)

.PHONY: tidy
tidy:
	clang-tidy $(TIDYFLAGS) $(ALL_SRC)

$(COMMON_LIB):
	$(MAKE) -C common COMMON_OUT_DIR=$(COMMON_OUT_DIR)

.PHONY: luna
luna: bin/luna
bin/luna: $(LUNA_OBJECTS) $(COMMON_LIB)
	@$(LD) $(LDFLAGS) $(LUNA_OBJECTS) $(COMMON_LIB) -o bin/luna

.PHONY: clean
clean:
	@rm -rf build/
	@rm -rf bin/
	@mkdir build/
	@mkdir bin/
	@mkdir -p $(dir $(LUNA_OBJECTS))
	@mkdir -p $(COMMON_OUT_DIR)

-include $(LUNA_OBJECTS:.o=.d)

# generate compile commands with bear if u got it!!! 
# very good highly recommended ʕ·ᴥ·ʔ
.PHONY: bear-gen-cc
bear-gen-cc: clean
	bear -- $(MAKE) luna
