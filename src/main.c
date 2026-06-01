#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h"
#include <stdlib.h>
#include "tests.h"
#include <string.h>

#define UPLOAD_MAX_BYTES (8 * 1024 * 1024)

extern int opterr;

int main(int argc, char **argv){
    char *list_file = "speedtest_server_list.json";

    CURLcode global_status = curl_global_init(CURL_GLOBAL_ALL);
    if(global_status != CURLE_OK){
        perror("Klaida su curl: ");
        return -1;
    }

    char *user_country;
    find_user_loc(&user_country);
    printf("Naudotojo vietove: %s\n", user_country);

    int option;
    opterr = 0;

    while((option = getopt(argc, argv, "ldu")) != -1)
    {
        char *arg = argv[2];
        switch (option)
        {
        case 'l':
            location_test(list_file, arg);
            return 0;
        
        case 'd':
            download_test(arg);
            return 0;
        case 'u':
            upload_test(arg);
            return 0;
        case '?':
            printf("Tokia opcija neegzistuoja, paleidziamas automatizuotas testas\n");
            location_test(list_file, user_country);
            return 0;
        default:
            return 0;
        }
    }
    
    curl_global_cleanup();

    return 0;
}