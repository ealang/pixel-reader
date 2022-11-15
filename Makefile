PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
	PLATFORM=linux
endif

PREFIX ?= /usr

COMMON_CXXFLAGS := -pedantic-errors -Wall -Wextra -std=c++17 -O2

ifeq ($(PLATFORM),miyoomini)
CXXFLAGS := $(COMMON_CXXFLAGS) -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd
else
CXXFLAGS := $(COMMON_CXXFLAGS)
endif

CXX      := $(CROSS_COMPILE)c++
LDFLAGS  := -L/usr/lib -lstdc++ -lm -lzip -lxml2 -lSDL -lSDL_ttf -lSDL_image
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)
INCLUDE  := -Isrc -I${PREFIX}/include/libxml2

COMMON_SRC := $(filter-out src/reader/main.cpp, $(wildcard src/epub/*.cpp src/reader/*.cpp src/sys/*.cpp))
READER_SRC := $(COMMON_SRC) src/reader/main.cpp
SANDBOX_SRC := $(COMMON_SRC) $(wildcard src/sandbox/*.cpp)
TEST_SRC := $(COMMON_SRC) $(wildcard src/sys/tests/*.cpp src/reader/tests/*.cpp)

APP_READER_TARGET := reader
APP_SANDBOX_TARGET := sandbox
APP_TEST_TARGET := test

READER_OBJECTS  := $(READER_SRC:%.cpp=$(OBJ_DIR)/%.o)
SANDBOX_OBJECTS := $(SANDBOX_SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJECTS    := $(TEST_SRC:%.cpp=$(OBJ_DIR)/%.o)

DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(APP_DIR)/$(APP_READER_TARGET) $(APP_DIR)/$(APP_SANDBOX_TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(APP_READER_TARGET): $(READER_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(APP_DIR)/$(APP_SANDBOX_TARGET): $(SANDBOX_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(APP_DIR)/$(APP_TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lgtest -lgtest_main

-include $(DEPENDENCIES)

.PHONY: all build clean debug release run_tests

test: $(APP_DIR)/$(APP_TEST_TARGET)
	$(APP_DIR)/$(APP_TEST_TARGET)

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

clean:
	-@rm -rf $(BUILD)
