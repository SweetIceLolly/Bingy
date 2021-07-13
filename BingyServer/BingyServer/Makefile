CC = g++
CFLAGS = -std=c++17 -Wall -O2
BUILD_DIR = build
SRC_DIR = src
LIBS = -L/usr/local/lib -lmongocxx -lbsoncxx -lpthread
INCLUDES = -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi

build: bingy.o config_parser.o database.o equipment.o game.o http_auth.o http_handlers.o http_router.o init.o monster.o player.o signin_event.o synthesis.o thread_pool.o trade.o utils.o mongoose.o rest_server.o
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIBS) -o $(BUILD_DIR)/bingy $(BUILD_DIR)/bingy.o $(BUILD_DIR)/config_parser.o $(BUILD_DIR)/database.o $(BUILD_DIR)/equipment.o $(BUILD_DIR)/game.o $(BUILD_DIR)/http_auth.o $(BUILD_DIR)/http_handlers.o $(BUILD_DIR)/http_router.o $(BUILD_DIR)/init.o $(BUILD_DIR)/monster.o $(BUILD_DIR)/player.o $(BUILD_DIR)/signin_event.o $(BUILD_DIR)/synthesis.o $(BUILD_DIR)/thread_pool.o $(BUILD_DIR)/trade.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/mongoose.o $(BUILD_DIR)/rest_server.o

bingy.o: $(SRC_DIR)/bingy.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/bingy.o $(SRC_DIR)/bingy.cpp

config_parser.o: $(SRC_DIR)/config_parser.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/config_parser.o $(SRC_DIR)/config_parser.cpp

database.o: $(SRC_DIR)/database.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/database.o $(SRC_DIR)/database.cpp

equipment.o: $(SRC_DIR)/equipment.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/equipment.o $(SRC_DIR)/equipment.cpp

game.o: $(SRC_DIR)/game.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/game.o $(SRC_DIR)/game.cpp

http_auth.o: $(SRC_DIR)/http_auth.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/http_auth.o $(SRC_DIR)/http_auth.cpp

http_handlers.o: $(SRC_DIR)/http_handlers.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/http_handlers.o $(SRC_DIR)/http_handlers.cpp

http_router.o: $(SRC_DIR)/http_router.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/http_router.o $(SRC_DIR)/http_router.cpp

init.o: $(SRC_DIR)/init.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/init.o $(SRC_DIR)/init.cpp

monster.o: $(SRC_DIR)/monster.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/monster.o $(SRC_DIR)/monster.cpp

player.o: $(SRC_DIR)/player.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/player.o $(SRC_DIR)/player.cpp

signin_event.o: $(SRC_DIR)/signin_event.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/signin_event.o $(SRC_DIR)/signin_event.cpp

synthesis.o: $(SRC_DIR)/synthesis.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/synthesis.o $(SRC_DIR)/synthesis.cpp

thread_pool.o: $(SRC_DIR)/thread_pool.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/thread_pool.o $(SRC_DIR)/thread_pool.cpp

trade.o: $(SRC_DIR)/trade.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/trade.o $(SRC_DIR)/trade.cpp

utils.o: $(SRC_DIR)/utils.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/utils.o $(SRC_DIR)/utils.cpp

mongoose.o: $(SRC_DIR)/rest_server/mongoose.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/mongoose.o $(SRC_DIR)/rest_server/mongoose.c

rest_server.o: $(SRC_DIR)/rest_server/rest_server.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c -o $(BUILD_DIR)/rest_server.o $(SRC_DIR)/rest_server/rest_server.cpp

clean:
	rm -f $(BUILD_DIR)/bingy $(BUILD_DIR)/bingy.o $(BUILD_DIR)/config_parser.o $(BUILD_DIR)/database.o $(BUILD_DIR)/equipment.o $(BUILD_DIR)/game.o $(BUILD_DIR)/http_auth.o $(BUILD_DIR)/http_handlers.o $(BUILD_DIR)/http_router.o $(BUILD_DIR)/init.o $(BUILD_DIR)/monster.o $(BUILD_DIR)/player.o $(BUILD_DIR)/signin_event.o $(BUILD_DIR)/synthesis.o $(BUILD_DIR)/thread_pool.o $(BUILD_DIR)/trade.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/mongoose.o $(BUILD_DIR)/rest_server.o
	rmdir $(BUILD_DIR)
