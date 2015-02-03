#Supplementary functions
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

#Variables
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
TEST_SRC_DIR = tests
TEST_BIN_DIR = bin/tests
TEST_OBJ_DIR = obj/tests

CC       = gcc
LINKER   = $(CC) -o
RM       = rm -rf
MKPATH   = mkdir -p
STRIP    = /usr/bin/strip

LFLAGS         = -Wall -Wextra -pedantic -lpng -lz
LFLAGS_DEBUG   =
LFLAGS_RELEASE =
CFLAGS         = -Wall -Wextra -pedantic -Wwrite-strings -Wsuggest-attribute=const -Wunused -Wc++-compat -std=c99 -iquote $(SRC_DIR)
CFLAGS_DEBUG   = -ggdb -DENABLE_ASSERT
CFLAGS_RELEASE = -Os

#OS specific linker settings
SYSTEM := $(shell $(CC) -dumpmachine)
ifneq (, $(findstring cygwin, $(SYSTEM)))
	LFLAGS += -liconv
endif
ifneq (, $(findstring mingw32, $(SYSTEM)))
	LFLAGS += -liconv
endif



#General targets
.PHONY: all
all: release debug tests

.PHONY: release
release: CFLAGS += $(CFLAGS_RELEASE)
release: LFLAGS += $(LFLAGS_RELEASE)
release: release_binaries

.PHONY: debug
debug: CFLAGS += $(CFLAGS_DEBUG)
debug: LFLAGS += $(LFLAGS_DEBUG)
debug: debug_binaries



#Binaries
RELEASE_SOURCES := $(filter-out $(SRC_DIR)/bin%.c $(SRC_DIR)/test_support%.c, $(call rwildcard, $(SRC_DIR)/, *.c))
DEBUG_SOURCES := $(filter-out $(SRC_DIR)/bin%.c, $(call rwildcard, $(SRC_DIR)/, *.c))
RELEASE_OBJECTS := $(RELEASE_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/release/%.o)
DEBUG_OBJECTS := $(DEBUG_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/debug/%.o)

.PHONY: release_binaries debug_binaries
release_binaries: $(BIN_DIR)/release/arc_unpacker $(BIN_DIR)/release/file_decoder
	@if [ -f "$(STRIP)" ]; then $(STRIP) --strip-all $(BIN_DIR)/release/*; fi
debug_binaries: $(BIN_DIR)/debug/arc_unpacker $(BIN_DIR)/debug/file_decoder

$(BIN_DIR)/debug/%: $(OBJ_DIR)/debug/bin/%.o $(DEBUG_OBJECTS)
	@$(MKPATH) $(dir $@)
	@echo Linking $@
	@$(LINKER) $@ $^ $(LFLAGS)

$(BIN_DIR)/release/%: $(OBJ_DIR)/release/bin/%.o $(RELEASE_OBJECTS)
	@$(MKPATH) $(dir $@)
	@echo Linking $@
	@$(LINKER) $@ $^ $(LFLAGS)

$(OBJ_DIR)/release/%.o: $(SRC_DIR)/%.c
	@$(MKPATH) $(dir $@)
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.c
	@$(MKPATH) $(dir $@)
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@



#Tests
TEST_SOURCES := $(call rwildcard, $(TEST_SRC_DIR)/, *.c)
TEST_BINARIES := $(TEST_SOURCES:$(TEST_SRC_DIR)/%.c=$(TEST_BIN_DIR)/%)

.PHONY: tests
tests: CFLAGS += $(CFLAGS_DEBUG)
tests: LFLAGS += $(LFLAGS_DEBUG)
tests: $(TEST_BINARIES)
	@$(foreach x,$^,echo Running tests: $(x);$(x);)

$(TEST_BIN_DIR)/%: $(TEST_OBJ_DIR)/%.o $(DEBUG_OBJECTS)
	@$(MKPATH) $(dir $@)
	@echo Linking $@
	@$(LINKER) $@ $^ $(LFLAGS)

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	@$(MKPATH) $(dir $@)
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@



#Additional targets
.PHONY: clean
clean:
	$(RM) ./$(BIN_DIR)/*
	$(RM) ./$(OBJ_DIR)/*


#Disable removing .o after successful build
.SECONDARY:
#Keep binaries of failed tests for gdb
.PRECIOUS: $(TEST_BIN_DIR)/%
