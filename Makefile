rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CXX      = g++
LINKER   = $(CXX) -o
RM       = rm -rf
MKPATH   = mkdir -p
STRIP    = /usr/bin/strip

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
TEST_SRC_DIR = tests

LFLAGS = -Wall -Wextra -pedantic -lpng -lz -lstdc++ -lboost_filesystem -lboost_locale -lboost_system
CXXFLAGS = -Wall -Wextra -pedantic -Wwrite-strings -Wno-unused-variable -Wno-unused-parameter -std=c++11 -iquote $(SRC_DIR)

ifeq (${DEBUG}, 1)
	CXXFLAGS += -ggdb -DDEBUG
	BIN_DIR := $(BIN_DIR)/debug
	OBJ_DIR := $(OBJ_DIR)/debug
else
	CXXFLAGS += -Os -DNDEBUG
	BIN_DIR := $(BIN_DIR)/release
	OBJ_DIR := $(OBJ_DIR)/release
endif
TEST_BIN_DIR := $(BIN_DIR)/tests
TEST_OBJ_DIR := $(OBJ_DIR)/tests

#OS specific linker settings
SYSTEM := $(shell $(CXX) -dumpmachine)
ifneq (, $(findstring cygwin, $(SYSTEM)))
	LFLAGS += -liconv
endif
ifneq (, $(findstring mingw32, $(SYSTEM)))
	LFLAGS += -liconv
endif



#General targets
.PHONY: all
all: binaries tests

#Main binaries
SOURCES := $(filter-out $(SRC_DIR)/bin/%.cc, $(call rwildcard, $(SRC_DIR)/, *.cc))
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)

.PHONY: binaries
binaries: $(BIN_DIR)/arc_unpacker $(BIN_DIR)/file_decoder

$(BIN_DIR)/%: $(OBJ_DIR)/bin/%.o $(OBJECTS)
	@$(MKPATH) $(dir $@)
	@echo Linking $@
	@$(LINKER) $@ $^ $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@$(MKPATH) $(dir $@)
	@echo Compiling $@
	@$(CXX) $(CXXFLAGS) -c $< -o $@

#Tests
TEST_SOURCES := $(call rwildcard, $(TEST_SRC_DIR)/, *.cc)
TEST_BINARIES := $(TEST_SOURCES:$(TEST_SRC_DIR)/%.cc=$(TEST_BIN_DIR)/%)

.PHONY: tests
tests: $(TEST_BINARIES)
	@$(foreach x,$^,echo Running tests: $(x);$(x);)

$(TEST_BIN_DIR)/%: $(TEST_OBJ_DIR)/%.o $(OBJECTS)
	@$(MKPATH) $(dir $@)
	@echo Linking $@
	@$(LINKER) $@ $^ $(LFLAGS)

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cc
	@$(MKPATH) $(dir $@)
	@echo Compiling $@
	@$(CXX) $(CXXFLAGS) -c $< -o $@



#Additional targets
.PHONY: clean
clean:
	$(RM) ./$(BIN_DIR)/*
	$(RM) ./$(OBJ_DIR)/*
	$(RM) ./$(TEST_BIN_DIR)/*
	$(RM) ./$(TEST_OBJ_DIR)/*


#Disable removing .o after successful build
.SECONDARY:
#Keep binaries of failed tests for gdb
.PRECIOUS: $(TEST_BIN_DIR)/%
