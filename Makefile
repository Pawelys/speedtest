CC = gcc
CFLAGS = -Wall -Wextra -g
CURL_CFLAGS = $(shell curl-config --cflags)
CURL_LIBS = $(shell curl-config --libs)

CJSON_DIR = ./cJSON

main: main.c
	$(CC) $(CFLAGS) $(CURL_CFLAGS) -I$(CJSON_DIR) main.c $(CJSON_DIR)/cJSON.c -o main $(CURL_LIBS)

clean:
	rm -f main