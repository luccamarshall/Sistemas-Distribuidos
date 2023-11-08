BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
DEP_DIR = dependencies

CC = gcc
CFLAGS = -g -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR)
LDFLAGS = -lprotobuf-c

KEEP_OBJECTS = $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/data.o

EXECS = $(BIN_DIR)/table-client $(BIN_DIR)/table-server

all: libtable $(EXECS)

libtable: $(LIB_DIR)/libtable.a

table-client: $(BIN_DIR)/table-client

table-server: $(BIN_DIR)/table-server

$(LIB_DIR)/libtable.a: $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
    ar -rcs $@ $^

$(BIN_DIR)/table-client: $(OBJ_DIR)/table_client.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(LIB_DIR)/libtable.a
    $(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/table-server: $(OBJ_DIR)/table_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/message.o $(OBJ_DIR)/sdmessage.pb-c.o $(LIB_DIR)/libtable.a
    $(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -rf $(DEP_DIR)/* $(filter-out $(KEEP_OBJECTS), $(wildcard $(OBJ_DIR)/*.o)) $(BIN_DIR)/* $(LIB_DIR)/*