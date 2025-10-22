# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wformat=0 -Wcast-align -Wnull-dereference -g3 -O0

# Platform-specific settings
ifeq ($(OS),Windows_NT)
	EXECUTABLE = outDebug.exe
	LDFLAGS = -lSDL2 -lSDL2_mixer -lpthread
	OPENMP = -fopenmp
	RM = rm -f
	MKDIR = mkdir -p $(1)
# MKDIR = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
	RUN_CMD = .\$(TARGET)
else
	EXECUTABLE = outDebug
	ifeq ($(shell uname -s),Darwin)
		# macOS specific flags
		LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lSDL2 -lSDL2_mixer -lpthread -lomp
		# Add SDL2 include paths for macOS
		CFLAGS += -I/opt/homebrew/include -I/usr/local/include
		# Set OpenMP flags for macOS/clang
		OPENMP = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib
	else
		# Linux specific flags
		LDFLAGS = -lSDL2 -lSDL2_mixer -lpthread -lm
		OPENMP = -fopenmp
	endif
	RM = rm -f
	MKDIR = mkdir -p $(1)
	RUN_CMD = ./$(TARGET)
endif

# Directories
BUILD_DIR = build/Debug
SRC_DIR = .

# Target executable
TARGET = $(BUILD_DIR)/$(EXECUTABLE)

# Source files (all .c files in current directory)
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

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
	$(RUN_CMD)

# Run the executable
debug: $(TARGET)
	$(RUN_CMD) -debug

# Debug with valgrind (for Linux/macOS)
run_debug: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=definite --track-origins=yes -s $(TARGET)

# Clean build files
clean:
	$(RM) $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all run run_debug clean