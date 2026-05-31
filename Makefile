CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Werror
CURL_CFLAGS = $(shell curl-config --cflags)
CURL_LIBS = $(shell curl-config --libs)

SRC_DIR = ./src
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/tests.c
CJSON_DIR = ./cJSON

main: $(SRCS)
	$(CC) $(CURL_CFLAGS) -I$(CJSON_DIR) $(SRCS) $(CJSON_DIR)/cJSON.c -o main $(CURL_LIBS)

clean:
	rm -f main