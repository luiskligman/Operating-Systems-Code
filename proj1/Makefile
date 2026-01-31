# Makefile - Project 1 (student)
#
# Builds:
#   src/main.cc  -> build/main.o  -> bin/proj1
#
# main.cc depends on include/main.h
# Links against instructor-provided static library:
#   lib/proj1lib.a
#
# Usage:
#   make
#   make clean

CXX      := g++
RM       := rm -f

# Flags
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g -I.. -pthread
LDFLAGS  := -pthread

# Paths / targets
BIN_DIR := bin
BUILD_DIR := build

BIN := $(BIN_DIR)/proj1
OBJ := $(BUILD_DIR)/main.o
LIB := lib/proj1lib.a

.PHONY: all clean

all: $(BIN)

# Link step (object first, then static library)
$(BIN) : $(OBJ)
	$(CXX) -o $@ $^ $(LIB) $(LDFLAGS)

# Compile step
$(OBJ) : src/main.cc include/main.h lib/proj1lib.a
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Cleanup
clean:
	$(RM) $(OBJ) $(BIN)

