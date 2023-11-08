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
