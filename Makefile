CC = gcc
CFLAGS = -Wall -Wextra -g
CURL_CFLAGS = $(shell curl-config --cflags)
CURL_LIBS = $(shell curl-config --libs)

SRC_DIR = ./src
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/results.c
CJSON_DIR = ./cJSON

main: $(SRCS)
	$(CC) $(CURL_CFLAGS) -I$(CJSON_DIR) $(SRCS) $(CJSON_DIR)/cJSON.c -o main $(CURL_LIBS)

clean:
	rm -f main