# BIN_DIR = binary
# INC_DIR = include
# LIB_DIR = lib
# OBJ_DIR = object
# SRC_DIR = source
# DEP_DIR = dependencies

# CC = gcc
# CFLAGS = -g -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR)
# LDFLAGS = -lprotobuf-c

# KEEP_OBJECTS = $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/data.o

# EXECS = $(BIN_DIR)/table-client $(BIN_DIR)/table-server

# all: libtable $(EXECS)

# libtable: $(LIB_DIR)/libtable.a

# table-client: $(BIN_DIR)/table-client

# table-server: $(BIN_DIR)/table-server

# $(LIB_DIR)/libtable.a: $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
#     ar -rcs $@ $^

# $(BIN_DIR)/table-client: $(OBJ_DIR)/table_client.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(LIB_DIR)/libtable.a
#     $(CC) $^ -o $@ $(LDFLAGS)

# $(BIN_DIR)/table-server: $(OBJ_DIR)/table_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/message.o $(OBJ_DIR)/sdmessage.pb-c.o $(LIB_DIR)/libtable.a
#     $(CC) $^ -o $@ $(LDFLAGS)

# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
#     $(CC) $(CFLAGS) -c $< -o $@

# clean:
#     rm -rf $(DEP_DIR)/* $(filter-out $(KEEP_OBJECTS), $(wildcard $(OBJ_DIR)/*.o)) $(BIN_DIR)/* $(LIB_DIR)/*

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -I./include

# Source and object directories
SRCDIR = ./source
OBJDIR = ./object
LIBDIR = ./lib
BINDIR = ./binary

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Object files
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Client and server object files
BINARY = $(BINDIR)/table-client $(BINDIR)/table-server

CLIENT_OBJECTS = $(filter-out $(OBJDIR)/table_server.o $(OBJDIR)/table_skel.o $(OBJDIR)/network_server.o, $(OBJECTS))
SERVER_OBJECTS = $(filter-out $(OBJDIR)/table_client.o $(OBJDIR)/client_stub.o $(OBJDIR)/network_client.o, $(OBJECTS))

# Library
LIBRARY = $(LIBDIR)/libtable.a

# Create object and library directories if they don't exist
$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(LIBDIR))
$(shell mkdir -p $(BINDIR))

# Targets
all: libtable table-client table-server

libtable: $(OBJECTS)
	$(AR) -rcs $(LIBRARY) $^

table-client: $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $(BINDIR)/table_client $^ -L$(LIBDIR) -ltable -lprotobuf-c -lpthread

table-server: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $(BINDIR)/table_server $^ -L$(LIBDIR) -ltable -lprotobuf-c -lpthread

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f table-client table-server $(LIBRARY) $(OBJECTS)
	rm -rf $(BINDIR)/*
