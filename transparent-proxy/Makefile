CC = g++  # Compiler
CFLAGS = -std=c++17  # Compiler flags

concurrency = src/concurrency/*.cpp
http = src/http/*.cpp
logger = src/logger/*.cpp
net = src/net/*.cpp
utils = src/utils/*.cpp

all:
	$(CC) $(CFLAGS) $(concurrency) $(http) $(logger) $(net) $(utils) src/server.cpp -o server

clean:
	rm -f $(server)

.PHONY: all clean