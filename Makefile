PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
	PLATFORM=linux
endif

PREFIX ?= /usr

COMMON_CXXFLAGS := -pedantic-errors -Wall -Wextra

ifeq ($(PLATFORM),miyoomini)
CXXFLAGS := $(COMMON_CXXFLAGS) -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd
else
CXXFLAGS := $(COMMON_CXXFLAGS)
endif

CXX      := $(CROSS_COMPILE)c++
LDFLAGS  := -L/usr/lib -lstdc++ -lm -lzip -lxml2 -lSDL -lSDL_ttf -lSDL_image
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/app
INCLUDE  := -Isrc -I${PREFIX}/include/libxml2

COMMON_SRC := $(wildcard src/epub/*.cpp src/sys/*.cpp)
READER_SRC := $(COMMON_SRC) $(wildcard src/reader/*.cpp)
SANDBOX_SRC := $(COMMON_SRC) src/sandbox_main.cpp

APP_READER_TARGET := reader
APP_SANDBOX_TARGET := sandbox

READER_OBJECTS  := $(READER_SRC:%.cpp=$(OBJ_DIR)/%.o)
SANDBOX_OBJECTS := $(SANDBOX_SRC:%.cpp=$(OBJ_DIR)/%.o)

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

-include $(DEPENDENCIES)

.PHONY: all build clean debug release

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rf $(BUILD)
