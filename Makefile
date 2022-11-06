# Credit https://www.partow.net/programming/makefile/index.html
CXX      := -c++
CXXFLAGS := -pedantic-errors -Wall -Wextra -I/usr/include/libxml2
LDFLAGS  := -L/usr/lib -lstdc++ -lm -lzip -lxml2
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/app
INCLUDE  := -Iinclude/
SRC      :=                      \
   $(wildcard src/*.cpp)         \

APP_TEST_TARGET := test
APP_READER_TARGET := reader

OBJECTS     := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
LIB_OBJECTS := $(filter-out $(OBJ_DIR)/src/%_main.o, $(OBJECTS))
EPUB_TEST_OBJECTS := $(LIB_OBJECTS) $(OBJ_DIR)/src/test_main.o
EPUB_READER_OBJECTS := $(LIB_OBJECTS) $(OBJ_DIR)/src/reader_main.o

DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(APP_DIR)/$(APP_TEST_TARGET) $(APP_DIR)/$(APP_READER_TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(APP_TEST_TARGET): $(EPUB_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(APP_TEST_TARGET) $^ $(LDFLAGS)

$(APP_DIR)/$(APP_READER_TARGET): $(EPUB_READER_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(APP_READER_TARGET) $^ $(LDFLAGS)

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

index:
	ctags -R src \
		/home/elang/git/union-miyoomini-toolchain/workspace/imports/headers/libxml2 \
		/home/elang/git/union-miyoomini-toolchain/workspace/imports/headers/zip.h
