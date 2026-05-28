CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
CPPFLAGS := -Isrc/common

ifeq ($(OS),Windows_NT)
	EXE := .exe
	NET_LIBS := -lws2_32
else
	EXE :=
	NET_LIBS :=
endif

BIN_DIR := bin
SERVER_BIN := $(BIN_DIR)/transit_server$(EXE)
CLIENT_BIN := $(BIN_DIR)/transit_client$(EXE)

COMMON_SRCS := $(wildcard src/common/*.c)
SERVER_SRCS := $(COMMON_SRCS) $(wildcard src/server/*.c)
CLIENT_SRCS := $(COMMON_SRCS) $(wildcard src/client/*.c)
COMMON_HDRS := $(wildcard src/common/*.h)
SERVER_HDRS := $(wildcard src/server/*.h)
CLIENT_HDRS := $(wildcard src/client/*.h)

.PHONY: all server client data-routes data-subway clean dirs

all: dirs server client

dirs:
	mkdir -p $(BIN_DIR) build data client_data logs

server: $(SERVER_BIN)

client: $(CLIENT_BIN)

data-routes:
	python tools/generate_bus_route_data.py

data-subway:
	python tools/generate_subway_data.py

$(SERVER_BIN): $(SERVER_SRCS) $(COMMON_HDRS) $(SERVER_HDRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(SERVER_SRCS) $(NET_LIBS)

$(CLIENT_BIN): $(CLIENT_SRCS) $(COMMON_HDRS) $(CLIENT_HDRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(CLIENT_SRCS) $(NET_LIBS)

clean:
	rm -rf $(BIN_DIR) build
