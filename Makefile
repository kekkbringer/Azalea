TARGET_EXEC ?= ../bin/azalea

BUILD_DIR ?= obj
SRC_DIRS ?= .

CC = g++
CXX = g++

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d -not -path '*/.*' -not -path './obj' -not -path './bin')
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -O3 -std=c++17 -static -flto=auto
LDFLAGS ?= $(INC_FLAGS) -static -O3 -flto=auto

# executable
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
#	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	rm -r $(BUILD_DIR)
config:
	$(MKDIR_P) obj bin

-include $(DEPS)

MKDIR_P ?= mkdir -p
