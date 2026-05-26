#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h"


int main(void){
    curl_global_init(CURL_GLOBAL_ALL);



    curl_global_cleanup();

    return 0;
}