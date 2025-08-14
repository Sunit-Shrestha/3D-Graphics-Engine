# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Debug flags
DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -DNDEBUG

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
ASSETS_DIR = assets

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

# Target executable
TARGET = sfml_renderer

# Build mode (debug or release)
MODE ?= debug
ifeq ($(MODE), release)
    CXXFLAGS += $(RELEASE_FLAGS)
else
    CXXFLAGS += $(DEBUG_FLAGS)
endif

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create assets directory if it doesn't exist
$(ASSETS_DIR):
	mkdir -p $(ASSETS_DIR)

# Link the executable
$(TARGET): $(OBJECTS) | $(ASSETS_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files with dependency generation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPENDS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Release build
release:
	$(MAKE) MODE=release

# Debug build (default)
debug:
	$(MAKE) MODE=debug

# Install SFML (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install libsfml-dev

# Print variables for debugging
print-vars:
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "TARGET: $(TARGET)"

.PHONY: all clean rebuild run release debug install-deps print-vars