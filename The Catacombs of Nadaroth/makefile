# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wformat=0 -Wcast-align -Wnull-dereference -g3 -O0

# Platform-specific settings
ifeq ($(OS),Windows_NT)
	EXECUTABLE = outDebug.exe
	RELEASE_EXECUTABLE = outRelease.exe
	LDFLAGS = -lSDL2 -lSDL2_mixer -lpthread -lz
	OPENMP = -fopenmp
	RM = rm -f
	MKDIR = mkdir -p $(1)
# MKDIR = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
	RUN_CMD = .\$(TARGET)
else
	EXECUTABLE = outDebug
	RELEASE_EXECUTABLE = outRelease
	ifeq ($(shell uname -s),Darwin)
		# macOS specific flags
		LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lSDL2 -lSDL2_mixer -lpthread -lomp -lz
		# Add SDL2 include paths for macOS
		CFLAGS += -I/opt/homebrew/include -I/usr/local/include
		# Set OpenMP flags for macOS/clang
		OPENMP = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib
	else
		# Linux specific flags
		LDFLAGS = -lSDL2 -lSDL2_mixer -lpthread -lm -lz
		OPENMP = -fopenmp
	endif
	RM = rm -f
	MKDIR = mkdir -p $(1)
	RUN_CMD = ./$(TARGET)
endif

# Directories
BUILD_DIR = build/Debug
RELEASE_BUILD_DIR = build/Release
SRC_DIR = .

# Release flags (optimized, no debug symbols)
RELEASE_CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wformat=0 -Wcast-align -Wnull-dereference -O2 -DNDEBUG

# Target executable
TARGET = $(BUILD_DIR)/$(EXECUTABLE)

# Source files (all .c files in current directory)
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Release target and objects
RELEASE_TARGET = $(RELEASE_BUILD_DIR)/$(RELEASE_EXECUTABLE)
RELEASE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(RELEASE_BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OPENMP) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OPENMP) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	$(call MKDIR,$(BUILD_DIR))

# Run the executable
run: $(TARGET)
	$(RUN_CMD) $(filter-out run,$(MAKECMDGOALS)) $(ARGS)

# Run the executable with debug flag
debug: $(TARGET)
	$(RUN_CMD) -debug $(filter-out debug,$(MAKECMDGOALS)) $(ARGS)

# Prevent make from treating extra arguments as targets
%:
	@:

# Debug with valgrind (for Linux/macOS)
run_debug: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=definite --track-origins=yes -s $(TARGET)

# Release build
release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CFLAGS) $(OPENMP) -o $@ $^ $(LDFLAGS)

$(RELEASE_BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_BUILD_DIR)
	$(CC) $(RELEASE_CFLAGS) $(OPENMP) -c $< -o $@

$(RELEASE_BUILD_DIR):
	$(call MKDIR,$(RELEASE_BUILD_DIR))

# Run the release executable
run_release: $(RELEASE_TARGET)
ifeq ($(OS),Windows_NT)
	.\$(RELEASE_TARGET) $(ARGS)
else
	./$(RELEASE_TARGET) $(ARGS)
endif

# Clean build files
clean:
	$(RM) $(BUILD_DIR)/*.o $(TARGET)

clean_release:
	$(RM) $(RELEASE_BUILD_DIR)/*.o $(RELEASE_TARGET)

.PHONY: all run run_debug clean release run_release clean_release