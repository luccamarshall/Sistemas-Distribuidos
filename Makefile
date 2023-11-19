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

EXECS = proto $(BIN_DIR)/table-client $(BIN_DIR)/table-server

all: libtable $(EXECS)

proto: $(SRC_DIR)/sdmessage.pb-c.c $(INC_DIR)/sdmessage.pb-c.h

libtable: $(LIB_DIR)/libtable.a

table-client: proto $(BIN_DIR)/table-client

table-server: proto $(BIN_DIR)/table-server

$(SRC_DIR)/sdmessage.pb-c.c $(INC_DIR)/sdmessage.pb-c.h: sdmessage.proto | $(SRC_DIR) $(INC_DIR)
	protoc-c --c_out=$(SRC_DIR) $<
	mv $(SRC_DIR)/sdmessage.pb-c.h $(INC_DIR)/

$(LIB_DIR)/libtable.a: $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
	ar -rcs $@ $^

$(BIN_DIR)/table-client: $(OBJ_DIR)/table_client.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(OBJ_DIR)/semaphores.o $(LIB_DIR)/libtable.a 
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/table-server: $(OBJ_DIR)/table_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/message.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/semaphores.o $(LIB_DIR)/libtable.a
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

include $(wildcard $(DEP_DIR)/*.d)
clean:
	rm -rf $(DEP_DIR)/* $(filter-out $(KEEP_OBJECTS), $(wildcard $(OBJ_DIR)/*.o)) $(BIN_DIR)/* $(LIB_DIR)/* 