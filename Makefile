CLIENT_TARGET := client_app
SERVER_TARGET := server_app  

CC := gcc
CFLAGS = -g -Wall -Wextra -O2 

#Server
SERVER_SRC := src/daemon/daemon.c src/server/server.c src/netstats/netstats.c src/uci_loader/uci_loader.c 
INCLUDES := -I/usr/local/include
LIBS_DIR := -L/usr/local/lib
SERVER_LIBS := -lubox -luci -Wl,-rpath,/usr/local/lib

#Client
CLIENT_SRC := src/client/client.c 
SERVER_OBJS := $(SERVER_SRC:.c=.o)
 
.PHONY: all client daemon clean

all: $(CLIENT_TARGET) $(SERVER_TARGET) 
client: $(CLIENT_TARGET) 
server: $(SERVER_TARGET) 
clean: 
	 rm -f $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS_DIR) $(SERVER_LIBS)  
	

 







