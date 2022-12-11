PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
	PLATFORM=linux
endif

PREFIX ?= /usr

CXXFLAGS := -pedantic-errors -Wall -Wextra -std=c++17 -O2
LDFLAGS  := -lstdc++ -lSDL -lSDL_ttf -lSDL_image -lzip -lxml2 -lstdc++fs

ifeq ($(PLATFORM),miyoomini)
CXXFLAGS := $(CXXFLAGS) \
	    -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd \
	    -Icross-compile/miyoo-mini/include/libxml2 \
	    -Icross-compile/miyoo-mini/include \
	    -Wno-psabi  # silence "parameter passing for argument of type '...' changed in GCC 7.1" warnings
LDFLAGS := $(LDFLAGS) \
	-L$(PREFIX)/lib \
	-Lcross-compile/miyoo-mini/lib \
	-Wl,-rpath-link,cross-compile/miyoo-mini/lib
endif

CXX      := $(CROSS_COMPILE)c++
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)
INCLUDE  := -Isrc -I${PREFIX}/include/libxml2

COMMON_SRC := $(filter-out src/reader/main.cpp, $(wildcard src/epub/*.cpp src/reader/*.cpp src/reader/views/*.cpp src/sys/*.cpp src/util/*.cpp src/doc_api/*.cpp))
READER_SRC := $(COMMON_SRC) src/reader/main.cpp
SANDBOX_SRC := $(COMMON_SRC) $(wildcard src/sandbox/*.cpp)
TEST_SRC := $(COMMON_SRC) $(wildcard src/sys/tests/*.cpp src/reader/tests/*.cpp src/epub/tests/*.cpp src/util/tests/*.cpp src/doc_api/tests/*.cpp)

APP_READER_TARGET := reader
APP_SANDBOX_TARGET := sandbox
APP_TEST_TARGET := test

READER_OBJECTS  := $(READER_SRC:%.cpp=$(OBJ_DIR)/%.o)
SANDBOX_OBJECTS := $(SANDBOX_SRC:%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJECTS    := $(TEST_SRC:%.cpp=$(OBJ_DIR)/%.o)

DEPENDENCIES := \
	    $(READER_OBJECTS:.o=.d) \
	    $(SANDBOX_OBJECTS:.o=.d) \
	    $(TEST_OBJECTS:.o=.d)

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

.PHONY: all build clean debug release run_tests miyoo-mini-shell miyoo-mini-package

test: $(APP_DIR)/$(APP_TEST_TARGET)
	$(APP_DIR)/$(APP_TEST_TARGET)

miyoo-mini-shell:
	$(MAKE) -C cross-compile/miyoo-mini/union-miyoomini-toolchain shell WORKSPACE_DIR=$(shell pwd)

miyoo-mini-package:
	sh miyoo_mini_package.sh

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

clean:
	-@rm -rf $(BUILD)
