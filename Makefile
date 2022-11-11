# Credit https://www.partow.net/programming/makefile/index.html

PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
	PLATFORM=linux
endif

PREFIX ?= /usr

COMMON_CXXFLAGS := -pedantic-errors -Wall -Wextra -I${PREFIX}/include/libxml2

ifeq ($(PLATFORM),miyoomini)
CXXFLAGS := $(COMMON_CXXFLAGS) -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd -I${PREFIX}/include/libxml2 -DFONT_PATH='"/customer/app/Exo-2-Bold-Italic.ttf"'
else
CXXFLAGS := $(COMMON_CXXFLAGS) -DFONT_PATH='"fonts/Galmuri7_edit.ttf"'
endif

CXX      := $(CROSS_COMPILE)c++
LDFLAGS  := -L/usr/lib -lstdc++ -lm -lzip -lxml2 -lSDL -lSDL_ttf -lSDL_image
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/app
INCLUDE  := -Iinclude/
SRC      :=                      \
   $(wildcard src/*.cpp)         \

APP_TEST_TARGET := test
APP_READER_TARGET := reader
APP_SANDBOX_TARGET := sandbox

OBJECTS     := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
LIB_OBJECTS := $(filter-out $(OBJ_DIR)/src/%_main.o, $(OBJECTS))
EPUB_TEST_OBJECTS := $(LIB_OBJECTS) $(OBJ_DIR)/src/test_main.o
EPUB_READER_OBJECTS := $(LIB_OBJECTS) $(OBJ_DIR)/src/reader_main.o
SANDBOX_OBJECTS := $(LIB_OBJECTS) $(OBJ_DIR)/src/sandbox_main.o

DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(APP_DIR)/$(APP_TEST_TARGET) $(APP_DIR)/$(APP_READER_TARGET) $(APP_DIR)/$(APP_SANDBOX_TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(APP_TEST_TARGET): $(EPUB_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(APP_TEST_TARGET) $^ $(LDFLAGS)

$(APP_DIR)/$(APP_READER_TARGET): $(EPUB_READER_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(APP_READER_TARGET) $^ $(LDFLAGS)

$(APP_DIR)/$(APP_SANDBOX_TARGET): $(SANDBOX_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(APP_SANDBOX_TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean debug release info index

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -r $(BUILD)

info:
	@echo "[*] Application dir: ${APP_DIR}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Sources:         ${SRC}         "
	@echo "[*] Objects:         ${OBJECTS}     "
	@echo "[*] Dependencies:    ${DEPENDENCIES}"
