BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
DEP_DIR = dependencies
ZK_INCLUDE = /path/to/zookeeper/include
ZK_LIB = /path/to/zookeeper/lib
ZKFLAGS = -lzookeeper_mt

CC = gcc
CFLAGS = -g -Wall -I $(INC_DIR) -I $(ZK_INCLUDE)
LDFLAGS = -lprotobuf-c -L $(ZK_LIB) -lzookeeper_mt

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

$(OBJ_DIR)/table_skel-private.o: $(SRC_DIR)/table_skel-private.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/table-client: $(OBJ_DIR)/table_client.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(OBJ_DIR)/table_skel-private.o $(OBJ_DIR)/table-private.o $(OBJ_DIR)/stats.o $(LIB_DIR)/libtable.a 
	$(CC) $^ -o $@ $(LDFLAGS) $(ZKFLAGS)

$(BIN_DIR)/table-server: $(OBJ_DIR)/table_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/message.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/table_skel-private.o $(OBJ_DIR)/table-private.o $(OBJ_DIR)/stats.o $(LIB_DIR)/libtable.a
	$(CC) $^ -o $@ $(LDFLAGS) $(ZKFLAGS)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(DEP_DIR)/* $(OBJ_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*
