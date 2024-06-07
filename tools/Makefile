# Compiler
CC := g++

# Compile flags
CFLAGS := -std=c++17

# Source files
SRCS := custom_img_creator.cpp image_loader.cpp

# Executable names
EXECS := custom_img_creator image_loader

# Additional source file for custom_img_creator
LODEPNG_SRC := lodepng/lodepng.cpp

# Default target
all: $(EXECS)


# Define color codes for bold green and reset
BOLD_GREEN := \033[1;32m
RESET := \033[0m

# Rule for compiling custom_img_creator.cpp to custom_img_creator executable
custom_img_creator: custom_img_creator.cpp $(LODEPNG_SRC)
    $(CC) $(CFLAGS) $^ -o $@
    @echo "Compiled: $(BOLD_GREEN)./$@$(RESET)"

# Rule for compiling image_loader.cpp to image_loader executable
image_loader: image_loader.cpp
    $(CC) $(CFLAGS) $< -o $@
    @echo "Compiled: $(BOLD_GREEN)./$@$(RESET)"


# Phony target to clean up
.PHONY: clean
clean:
    rm -f $(EXECS) $(CUSTOM_EXE)
