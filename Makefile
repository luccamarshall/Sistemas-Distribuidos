# Directory structure
BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
DEP_DIR = dependencies

# Compiler and flags
CC = gcc
CFLAGS = -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR)

# Source files (add more if needed)
SRC_FILES = data.c entry.c list.c list-private.c serialization.c table.c table-private.c

# Test source files (add more if needed)
TEST_SRC_FILES = test_data.c test_entry.c test_list.c test_serialization.c test_table.c

# Generate object file names based on source files
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.c=.o))

# Generate object file names based on test source files
TEST_OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(TEST_SRC_FILES:.c=.o))

# Generate test program names
TEST_EXECS = $(addprefix $(BIN_DIR)/, $(TEST_SRC_FILES:.c=))

# Dependencies
DEP_FILES = $(addprefix $(DEP_DIR)/, $(SRC_FILES:.c=.d) $(TEST_SRC_FILES:.c=.d))

# Default target: Build all test programs
all: $(BIN_DIR) $(TEST_EXECS)

# Build test programs
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(OBJ_FILES)
	$(CC) $^ -o $@

# Build object files from source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build object files from test source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Include dependency files
-include $(wildcard $(DEP_FILES))

# Create directories
$(OBJ_DIR) $(DEP_DIR) $(BIN_DIR):
	mkdir -p $@

# Clean target: Remove generated files
clean:
	rm -rf $(DEP_DIR)/* $(OBJ_DIR)/* $(BIN_DIR)/*

.PHONY: all clean
