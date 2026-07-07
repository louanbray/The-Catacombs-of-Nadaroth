# ============================================================
# Compilers
# ============================================================
# For native builds
CC        = gcc

# For linux cross compile
CC_WIN    = x86_64-w64-mingw32-gcc

# Path to the mingw-w64 SDL2 / SDL2_mixer dev packages (edit this,
# or override on the command line: make release_windows MINGW_SDL2_ROOT=/path)


# ============================================================
# Shared warning flags (used by every build variant)
# ============================================================
WARN_FLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wcast-align -Wnull-dereference

# ============================================================
# Host platform detection (for the native debug/run targets)
# ============================================================
ifeq ($(OS),Windows_NT)
    EXECUTABLE = outDebug.exe
    RELEASE_EXECUTABLE = outRelease.exe
    PLATFORM_CFLAGS = -DWIN32 -D_WIN32 -D__USE_MINGW_ANSI_STDIO=1
    LDFLAGS = -lSDL2 -lSDL2_mixer -lpthread -lz
    OPENMP = -fopenmp
    RM = C:/msys64/usr/bin/rm -f
	RMDIR = C:/msys64/usr/bin/rm -rf
	CP = C:/msys64/usr/bin/cp
	ZIP = C:/msys64/usr/bin/zip
    MKDIR = if not exist $(subst /,\,$(patsubst %/,%,$(1))) mkdir $(subst /,\,$(patsubst %/,%,$(1)))
    RUN_CMD = .\$(TARGET)
    NATIVE_DIST_DIR = release/windows

	MINGW_SDL2_ROOT       ?= C:\msys64\mingw-w64\SDL2
	MINGW_SDL2_MIXER_ROOT ?= C:\msys64\mingw-w64\SDL2_mixer
	MINGW_ZLIB_ROOT       ?= C:\msys64\mingw-w64\zlib
else
    EXECUTABLE = outDebug
    RELEASE_EXECUTABLE = outRelease
    ifeq ($(shell uname -s),Darwin)
        # macOS specific flags
        PLATFORM_CFLAGS = -I/opt/homebrew/include -I/usr/local/include
        LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lSDL2 -lSDL2_mixer -lpthread -lomp -lz
        OPENMP = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib
        NATIVE_DIST_DIR = release/macos
    else
        # Linux specific flags
        PLATFORM_CFLAGS =
        LDFLAGS = -lSDL2 -lSDL2_mixer -lpthread -lm -lz
        OPENMP = -fopenmp
        NATIVE_DIST_DIR = release/linux
    endif
    RM = rm -f
	RMDIR = rm -rf
	CP = cp
	ZIP = zip
    MKDIR = mkdir -p $(1)
    RUN_CMD = ./$(TARGET)

	MINGW_SDL2_ROOT       ?= /opt/mingw-w64/SDL2
	MINGW_SDL2_MIXER_ROOT ?= /opt/mingw-w64/SDL2_mixer
	MINGW_ZLIB_ROOT       ?= /opt/mingw-w64/zlib
endif

CFLAGS = $(WARN_FLAGS) $(PLATFORM_CFLAGS) -g3 -O0

# ============================================================
# Directories
# ============================================================
SRC_DIR = src
BUILD_DIR = build/Debug
RELEASE_BUILD_DIR = build/Release
RELEASE_WIN_BUILD_DIR = build/Release-Windows

# ============================================================
# Release flags — native (Linux/macOS/Windows host), optimized
# ============================================================
RELEASE_CFLAGS = $(WARN_FLAGS) $(PLATFORM_CFLAGS) -Wformat=0 -O2 -DNDEBUG

# ============================================================
# Release flags — Windows cross-compile (via mingw-w64), always
# available regardless of host OS
# ============================================================
RELEASE_WIN_CFLAGS = $(WARN_FLAGS) -Wformat=0 -O2 -DNDEBUG -DWIN32 -D_WIN32 \
    -I$(MINGW_SDL2_ROOT)/include \
    -I$(MINGW_SDL2_ROOT)/include/SDL2 \
    -I$(MINGW_SDL2_MIXER_ROOT)/include \
    -I$(MINGW_SDL2_MIXER_ROOT)/include/SDL2 \
    -I$(MINGW_ZLIB_ROOT)/include

# -static-libgcc / winpthread statically linked so the .exe runs on a
# machine without mingw's runtime DLLs installed.
RELEASE_WIN_LDFLAGS = \
    -L$(MINGW_SDL2_ROOT)/lib \
    -L$(MINGW_SDL2_MIXER_ROOT)/lib \
    -L$(MINGW_ZLIB_ROOT)/lib \
    -static-libgcc -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic \
    -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lz -lwinmm -lole32 -loleaut32 -limm32 -lversion -lsetupapi

RELEASE_WIN_OPENMP = -fopenmp

# ============================================================
# Targets / sources (MODIFIED FOR RECURSION)
# ============================================================
# Fonction macro pour chercher récursivement les fichiers
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

TARGET = $(BUILD_DIR)/$(EXECUTABLE)
# Trouve tous les .c dans src/ et ses sous-dossiers
SRCS = $(call rwildcard,$(SRC_DIR),*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

RELEASE_TARGET = $(RELEASE_BUILD_DIR)/$(RELEASE_EXECUTABLE)
RELEASE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(RELEASE_BUILD_DIR)/%.o,$(SRCS))

RELEASE_WIN_TARGET = $(RELEASE_WIN_BUILD_DIR)/outRelease.exe
RELEASE_WIN_OBJS = $(patsubst $(SRC_DIR)/%.c,$(RELEASE_WIN_BUILD_DIR)/%.o,$(SRCS))

# ============================================================
# Default target (native debug build)
# ============================================================
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OPENMP) -o $@ $^ $(LDFLAGS)

# Changement ici : on crée dynamiquement le sous-dossier requis avant de compiler
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR,$(dir $@))
	$(CC) $(CFLAGS) $(OPENMP) -c $< -o $@

# ============================================================
# Run helpers
# ============================================================
run: $(TARGET)
	$(RUN_CMD) $(filter-out run,$(MAKECMDGOALS)) $(ARGS)

debug: $(TARGET)
	$(RUN_CMD) -debug $(filter-out debug,$(MAKECMDGOALS)) $(ARGS)

# Prevent make from treating extra arguments as targets
%:
	@:

run_debug: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=definite --track-origins=yes -s $(TARGET)

# ============================================================
# Native release build (Linux/macOS/Windows-host)
# ============================================================
release: $(RELEASE_TARGET)
	$(call MKDIR,$(NATIVE_DIST_DIR))
	$(CP) $(RELEASE_TARGET) $(NATIVE_DIST_DIR)/
	$(RMDIR) $(NATIVE_DIST_DIR)/assets
	$(CP) -r assets $(NATIVE_DIST_DIR)/
ifeq ($(NATIVE_DIST_DIR),release/windows)
	$(CP) $(MINGW_SDL2_ROOT)/bin/SDL2.dll $(NATIVE_DIST_DIR)/
	$(CP) $(MINGW_SDL2_MIXER_ROOT)/bin/SDL2_mixer.dll $(NATIVE_DIST_DIR)/
endif
	cd release && $(ZIP) -r $(notdir $(NATIVE_DIST_DIR)).zip $(notdir $(NATIVE_DIST_DIR))
	@echo "Deployed native release to $(NATIVE_DIST_DIR)/ and created $(notdir $(NATIVE_DIST_DIR)).zip"

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CFLAGS) $(OPENMP) -o $@ $^ $(LDFLAGS)

# Changement ici aussi pour la création de sous-dossier en release
$(RELEASE_BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR,$(dir $@))
	$(CC) $(RELEASE_CFLAGS) $(OPENMP) -c $< -o $@

run_release: $(RELEASE_TARGET)
ifeq ($(OS),Windows_NT)
	.\$(RELEASE_TARGET) $(ARGS)
else
	./$(RELEASE_TARGET) $(ARGS)
endif

# ============================================================
# Windows release build, cross-compiled from any host via mingw-w64
# ============================================================
release_windows: $(RELEASE_WIN_TARGET)
	$(call MKDIR,release/windows)
	$(CP) $(RELEASE_WIN_TARGET) release/windows/
	$(RMDIR) release/windows/assets
	$(CP) -r assets release/windows/
	$(CP) $(MINGW_SDL2_ROOT)/bin/SDL2.dll release/windows/
	$(CP) $(MINGW_SDL2_MIXER_ROOT)/bin/SDL2_mixer.dll release/windows/
	cd release && zip -r windows.zip windows
	@echo "Built and deployed bundle to release/windows/ and created windows.zip"

$(RELEASE_WIN_TARGET): $(RELEASE_WIN_OBJS)
	$(CC_WIN) $(RELEASE_WIN_CFLAGS) $(RELEASE_WIN_OPENMP) -o $@ $^ $(RELEASE_WIN_LDFLAGS)

# Changement ici aussi pour la cross-compilation Windows
$(RELEASE_WIN_BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR,$(dir $@))
	$(CC_WIN) $(RELEASE_WIN_CFLAGS) $(RELEASE_WIN_OPENMP) -c $< -o $@

# ============================================================
# Clean (MODIFIED TO CLEAN RECURSIVE OBJECTS PRECISELY)
# ============================================================
clean:
	$(RM) $(OBJS) $(TARGET)

clean_release:
	$(RM) $(RELEASE_OBJS) $(RELEASE_TARGET)
	$(RMDIR) release/linux release/macos release/windows
	$(RM) release/linux.zip release/macos.zip release/windows.zip

clean_release_windows:
	$(RM) $(RELEASE_WIN_OBJS) $(RELEASE_WIN_TARGET)
	$(RMDIR) release/windows
	$(RM) release/windows.zip

.PHONY: all run debug run_debug clean release run_release clean_release \
        release_windows clean_release_windows