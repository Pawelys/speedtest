#ifndef tests_h_
#define tests_h_
#include "cJSON.h"

struct result
{
    char *host;//serverio url
    double down;//Parsiuntimo greitis MB/s
    double up;  //Siuntimo greitis MB/s
};
void print_result(struct result result, char *header);
static size_t user_loc_callback(void *contents, size_t size, size_t nmemb, void *bufptr);
static size_t sink_callback(void *contents, size_t size, size_t nmemb, void *bufptr);
int find_user_loc(char **country);
int open_json(char *file, cJSON **root);
int filtrate_loc(const int count, const cJSON *root, const char *country, struct result *results);
int download_speed_test_all(struct result *servers, int count, struct result *best, FILE **payload);
double download_speed_test(struct result *server, FILE **payload);
double upload_speed_test(struct result *server, FILE *payload);
static size_t read_callback(void *contents, size_t size, size_t nmemb, void *bufptr);
int upload_speed_test_all(struct result *servers, int count, struct result *best, FILE *payload);

#endif