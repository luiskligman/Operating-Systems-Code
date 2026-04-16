# Makefile - Project 2 (student/test)
#
# Builds:
#   src/proj2_server.cc -> build/proj2_server.o -> bin/proj2-server
#
# proj2_server.cc may include headers from include/ and lib/include/
# Links against instructor-provided static library:
#   lib/proj2lib.a
#
# Usage:
#   make
#   make clean

CXX      := g++
RM       := rm -f
MKDIR    := mkdir -p

# Flags
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -g -Iinclude -Ilib/include -I.. -pthread
LDFLAGS  := -pthread

# Paths / targets
BIN_DIR   := bin
BUILD_DIR := build

BIN := $(BIN_DIR)/proj2-server
OBJ := $(BUILD_DIR)/proj2_server.o
DEP := $(BUILD_DIR)/proj2_server.d
LIB := lib/proj2lib.a

.PHONY: all clean

all: $(BIN) execs 

# Link step (object first, then static library)
$(BIN): $(OBJ) $(LIB) | $(BIN_DIR)
	$(CXX) -o $@ $< $(LIB) $(LDFLAGS)

execs:
	@chmod +x bin/proj2-client lib/util/server-parser

# Compile step with automatic dependency generation
$(BUILD_DIR)/%.o: src/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -c -o $@ $<

# Include dependency file if it exists
-include $(DEP)

# Cleanup
clean:
	$(RM) $(OBJ) $(DEP) $(BIN)
