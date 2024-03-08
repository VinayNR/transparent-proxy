CC = g++  # Compiler
CFLAGS = -std=c++17  # Compiler flags

concurrency = src/concurrency/*.cpp
logger = src/logger/*.cpp
netutils = src/netutils/*.cpp

all:
	$(CC) $(CFLAGS) $(concurrency) $(logger) $(netutils) src/server.cpp -o server

clean:
	rm -f $(server)

.PHONY: all clean