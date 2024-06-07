# Compiler
CC := g++

# Compile flags. For now we just switch off the warnings, to not to clutter the screen.
CFLAGS := -w -std=c++17

# SDL2 flags (using sdl2-config to get the proper flags for compilation and linking)
SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LDFLAGS := $(shell sdl2-config --libs)

# Find all CPP files recursively
SRCS := $(shell find . -type f -name '*.cpp')
# Generate executable names
EXECS := $(patsubst %.cpp,%,$(SRCS))

# Define color codes for bold green and reset
BOLD_GREEN := \033[1;32m
RESET := \033[0m

# Default target
all: $(EXECS)

# Rule for compiling CPP files to executables with SDL2 support
%: %.cpp
	@$(CC) $(CFLAGS) $(SDL2_CFLAGS) $< -o $@ $(SDL2_LDFLAGS)
	@echo "Compiled: $(BOLD_GREEN)./$@$(RESET)"


# Phony target to clean up
.PHONY: clean
clean:
	@rm -f $(EXECS)
	@echo "Cleaned"
