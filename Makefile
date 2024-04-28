PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
	PLATFORM=linux
endif

PREFIX ?= /usr

WARNFLAGS := -pedantic-errors -Wall -Wextra
CXXFLAGS := -std=c++17 -O2
LDFLAGS  := -lstdc++ -lSDL -lSDL_ttf -lSDL_image -lzip -lxml2 -lstdc++fs

ifeq ($(PLATFORM),miyoomini)
CXXFLAGS := $(CXXFLAGS) \
	    -DPLATFORM_MIYOO_MINI=1 \
	    -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd \
	    -Icross-compile/miyoo-mini/include/libxml2 \
	    -Icross-compile/miyoo-mini/include \
	    -Wno-psabi  # silence "parameter passing for argument of type '...' changed in GCC 7.1" warnings
LDFLAGS := $(LDFLAGS) \
	-Lcross-compile/miyoo-mini/lib \
	-L$(PREFIX)/lib \
	-Wl,-rpath-link,cross-compile/miyoo-mini/lib
endif

CXX      := $(CROSS_COMPILE)c++
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)
INCLUDE  := -Isrc -I${SYSROOT}${PREFIX}/include/libxml2

ROTOZOOM_SRC := src/extern/rotozoom/SDL_rotozoom.c
COMMON_SRC   := $(filter-out src/reader/main.cpp, $(wildcard src/filetypes/*.cpp src/filetypes/txt/*.cpp src/filetypes/epub/*.cpp src/reader/*.cpp src/reader/views/*.cpp src/reader/views/token_view/*.cpp src/sys/*.cpp src/util/*.cpp src/doc_api/*.cpp src/extern/hash-library/*.cpp))
READER_SRC   := $(COMMON_SRC) src/reader/main.cpp
SANDBOX_SRC  := $(COMMON_SRC) $(wildcard src/sandbox/*.cpp)
TEST_SRC     := $(COMMON_SRC) $(wildcard src/sys/tests/*.cpp src/reader/tests/*.cpp src/filetypes/epub/tests/*.cpp src/util/tests/*.cpp src/doc_api/tests/*.cpp)

APP_READER_TARGET := reader
APP_SANDBOX_TARGET := sandbox
APP_TEST_TARGET := test

ROTOZOOM_OBJECT := $(OBJ_DIR)/SDL_rotozoom.o
READER_OBJECTS  := $(READER_SRC:%.cpp=$(OBJ_DIR)/%.o) $(ROTOZOOM_OBJECT)
SANDBOX_OBJECTS := $(SANDBOX_SRC:%.cpp=$(OBJ_DIR)/%.o) $(ROTOZOOM_OBJECT)
TEST_OBJECTS    := $(TEST_SRC:%.cpp=$(OBJ_DIR)/%.o) $(ROTOZOOM_OBJECT)

DEPENDENCIES := \
	    $(READER_OBJECTS:.o=.d)  \
	    $(SANDBOX_OBJECTS:.o=.d) \
	    $(TEST_OBJECTS:.o=.d)

all: build $(APP_DIR)/$(APP_READER_TARGET) $(APP_DIR)/$(APP_SANDBOX_TARGET)

$(ROTOZOOM_OBJECT): $(ROTOZOOM_SRC)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDE) $(WARNFLAGS) -c $< -MMD -o $@

$(APP_DIR)/$(APP_READER_TARGET): $(READER_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(APP_DIR)/$(APP_SANDBOX_TARGET): $(SANDBOX_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(APP_DIR)/$(APP_TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lgtest -lgtest_main

-include $(DEPENDENCIES)

.PHONY: all build clean debug release run_tests miyoo-mini-shell

test: $(APP_DIR)/$(APP_TEST_TARGET)
	$(APP_DIR)/$(APP_TEST_TARGET)

miyoo-mini-shell:
	-$(MAKE) -C cross-compile/miyoo-mini/union-miyoomini-toolchain shell WORKSPACE_DIR=$(shell pwd)

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -g3 -O0
debug: all

debug_test: CXXFLAGS += -g
debug_test: test

clean:
	-@rm -rf $(BUILD)
