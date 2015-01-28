#Supplementary functions
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

#Variables
SRC_DIR   = src
OBJ_DIR   = obj
BIN_DIR   = bin
TEST_SRC_DIR = tests
TEST_OBJ_DIR = obj/tests
TEST_BIN_DIR = bin/tests

CC       = gcc
LINKER   = gcc -o
RM       = rm -rf
MKPATH   = mkdir -p
STRIP    = /usr/bin/strip

CFLAGS   = -Wall -Wextra -pedantic -O2 -std=gnu99 -I$(SRC_DIR)
LFLAGS   = -Wall -Wextra -pedantic

.PHONY: set_test_flags
set_test_flags:
	$(eval CFLAGS:=$(filter-out -Os -O1 -O2 -O3,$(CFLAGS)) -ggdb)

#OS specific linker settings
SYSTEM := $(shell gcc -dumpmachine)
ifneq (, $(findstring cygwin, $(SYSTEM)))
	LFLAGS += -liconv
endif



#Binaries
SOURCES := $(filter-out $(SRC_DIR)/bin%.c, $(call rwildcard, $(SRC_DIR)/, *.c))
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: debug
debug: | set_test_flags all

.PHONY: all
all: $(BIN_DIR)/arc_unpacker $(BIN_DIR)/file_decoder

$(BIN_DIR)/%: $(OBJ_DIR)/bin/%.o $(OBJECTS)
	@$(MKPATH) $(dir $@)
	$(LINKER) $@ $^ $(LFLAGS)
	if [ -f "$(STRIP)" ]; then $(STRIP) $@; fi

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKPATH) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@



#Tests
TEST_SOURCES := $(call rwildcard, $(TEST_SRC_DIR)/, *.c)
TEST_BINARIES := $(TEST_SOURCES:$(TEST_SRC_DIR)/%.c=$(TEST_BIN_DIR)/%)

.PHONY: tests
tests: set_test_flags
tests: $(TEST_BINARIES)
	$^

$(TEST_BIN_DIR)/%: $(TEST_OBJ_DIR)/%.o $(OBJECTS)
	@$(MKPATH) $(dir $@)
	$(LINKER) $@ $^ $(LFLAGS)

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	@$(MKPATH) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@



#Additional targets
.PHONY: clean
clean:
	$(RM) $(BIN_DIR)/*
	$(RM) $(OBJ_DIR)/*


#Disable removing .o after successful build
.SECONDARY:
#Keep binaries of failed tests for gdb
.PRECIOUS: $(TEST_BIN_DIR)/%
