MAIN_EXEC ?= azalea
TEST_EXEC ?= azalea_tests

BIN_DIR ?= bin
OBJ_DIR ?= obj
TEST_DIR ?= TESTS

CC = g++
CXX = g++

INCFLAGS ?= -I.

CPPFLAGS ?= $(INCFLAGS) -MMD -MP -O3 -std=c++17 -static -flto=auto -Wall
LDFLAGS ?= -static -O3 -flto=auto

# object files for main executable (excluding main itself)
OBJ_LIST := fen.o util.o statistics.o

# object files for test executable (excluding test main itself)
TOBJ_LIST := board_test.o

OBJS := $(foreach object,$(OBJ_LIST),$(OBJ_DIR)/$(object))
TOBJS := $(foreach object,$(TOBJ_LIST),$(OBJ_DIR)/TESTS/$(object))

all: $(BIN_DIR)/$(MAIN_EXEC) #$(BIN_DIR)/$(TEST_EXEC)

# main executable
$(BIN_DIR)/$(MAIN_EXEC): $(OBJS) $(OBJ_DIR)/main.o
	$(CC) $(OBJS) $(OBJ_DIR)/main.o -o ./$@ $(LDFLAGS)

# main object file
$(OBJ_DIR)/main.o: main.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# test executable
$(BIN_DIR)/$(TEST_EXEC): $(OBJS) $(TOBJS) $(OBJ_DIR)/unit_test.o
	$(CC) $(OBJS) $(TOBJS) $(OBJ_DIR)/unit_test.o -o ./$@ $(LDFLAGS)

# test object file
$(OBJ_DIR)/unit_test.o: $(TEST_DIR)/unit_test.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# c++ sources
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# c++ sources for unit tests
$(OBJ_DIR)/TESTS/%.o: $(TEST_DIR)%.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@


.PHONY: clean

clean:
	rm $(OBJ_DIR)/* $(BIN_DIR)/*
config:
	$(MKDIR_P) obj bin obj/TESTS
tests: $(BIN_DIR)/$(TEST_EXEC)

MKDIR_P ?= mkdir -p
