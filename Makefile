MAIN_EXEC ?= azalea
TEST_EXEC ?= azalea_tests

BIN_DIR ?= bin
OBJ_DIR ?= obj
TEST_DIR ?= TESTS

CC = g++
CXX = g++

INCFLAGS ?= -I.

CPPFLAGS ?= $(INCFLAGS) -MMD -MP -O3 -std=c++23 -Wall \
			-march=native
LDFLAGS ?= -O3

# object files for main executable (excluding main itself)
OBJ_LIST := fen.o util.o statistics.o movegenerator.o makeMove.o perft.o \
	    search.o qsearch.o unmakeMove.o zobrist.o

# object files for test executable (excluding test main itself)
TOBJ_LIST := board_test.o movegen_test.o perft_test.o

OBJS := $(foreach object,$(OBJ_LIST),$(OBJ_DIR)/$(object))
TOBJS := $(foreach object,$(TOBJ_LIST),$(OBJ_DIR)/TESTS/$(object))
DEPS := $(OBJS:%.o=%.d) $(TOBJS:%.o=%.d) \
	$(OBJ_DIR)/main.d $(OBJ_DIR)/TESTS/unit_tests.d


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

-include $(DEPS)


.PHONY: clean

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/* $(OBJ_DIR)/TESTS/*.o
	rm -f $(OBJ_DIR)/*.d $(BIN_DIR)/* $(OBJ_DIR)/TESTS/*.d
config:
	$(MKDIR_P) obj bin obj/TESTS
tests: $(BIN_DIR)/$(TEST_EXEC)

MKDIR_P ?= mkdir -p
