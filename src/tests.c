#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h"
#include <stdlib.h>
#include "tests.h"
#include <string.h>

#define UPLOAD_MAX_BYTES (8 * 1024 * 1024)

void print_result(const struct result result, char* header)
{
    printf("======================================\n");
    printf("|| %s ||\n", header);
    printf("======================================\n");
    printf("|| Download || Upload || Serveris ||\n");
    printf("======================================\n");
    printf("|| %.2fMbps || %.2fMbps || %s ||\n", result.down, result.up, result.host);
    printf("======================================\n");
}

int filtrate_loc(const int count, const cJSON *root, const char *country, struct result *results)
{   
    int accepted_count = 0;
    for(int i = 0; i < count; i++)
    {
        cJSON *entry = cJSON_GetArrayItem(root, i);
        char *host_country = cJSON_GetObjectItem(entry, "country")->valuestring;
        if(strcasecmp(host_country, country) == 0){
            results[accepted_count].host = cJSON_GetObjectItem(entry, "host")->valuestring;
            results[accepted_count].down = 0;
            results[accepted_count].up = 0;
            accepted_count += 1;
        }
    }
    if(accepted_count == 0){
        printf("Nerasta serveriu pagal vietove.\n");
        exit(255);
    }
    return accepted_count;
}

int download_speed_test_all(struct result *servers, int count, struct result *best, FILE **payload)
{
    for(int i = 0; i < count; i++)
    {
        double mbps = download_speed_test(&servers[i], payload);
        if(mbps > best->down){
            best->host = servers[i].host;
            best->down = servers[i].down;
        }
    }
    return 1;
}

double download_speed_test(struct result *server, FILE **payload)
{
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        fprintf(stderr, "Klaida inicializuojant CURL\n");
        return -1;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: curl/7.68.0");

    char url[512];
    snprintf(url, sizeof(url), "%s/speedtest/random4000x4000.jpg", server->host);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    if (payload != NULL) {
        if (*payload == NULL) {
            *payload = tmpfile();
            if (*payload == NULL) {
                fprintf(stderr, "Klaida: nepavyko sukurti laikino payload failo\n");
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                return -1;
            }
        }
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, *payload);
    } else {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sink_callback);
    }
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    long http_code = 0;
    curl_off_t bytes = 0;
    double total_time = 0.0;
    curl_off_t bps = 0;

    printf("Testuojama: %s || ", server->host);
    fflush(stdout);
    CURLcode ret = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &bytes);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
    curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &bps);

    if(ret != CURLE_OK){
        fprintf(stdout, "Klaida: %s\n", curl_easy_strerror(ret));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(http_code == 200 && bytes > 0){
        printf("OK (http:%ld bytes:%" CURL_FORMAT_CURL_OFF_T " time:%.2fs)\n", http_code, bytes, total_time);
        double mbps = (double)bps * 8.0 / 1000000.0;
        server->down = mbps;
        return mbps;
    }
    else
    {
        printf("Klaida: (http:%ld bytes:%" CURL_FORMAT_CURL_OFF_T " time:%.2fs)\n", http_code, bytes, total_time);
    }

    server->down = 0;
    return -1;
}

int upload_speed_test_all(struct result *servers, int count, struct result *best, FILE *payload)
{
    for(int i = 0; i < count; i++)
    {
        double mbps = upload_speed_test(&servers[i], payload);
        if(mbps > best->up){
            best->host = servers[i].host;
            best->up = servers[i].up;
        }
    }
    return 1;
}

double upload_speed_test(struct result *server, FILE *payload)
{
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        fprintf(stderr, "Klaida inicializuojant CURL\n");
        return -1;
    }

    if (payload == NULL) {
        fprintf(stderr, "Klaida: upload payload neegzistuoja\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    fflush(payload);
    fseek(payload, 0, SEEK_END);
    curl_off_t file_size = (curl_off_t)ftell(payload);
    if (file_size <= 0) {
        fprintf(stdout, "Klaida: netinkamas upload payload dydis\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_off_t upload_size = file_size;
    if (upload_size > UPLOAD_MAX_BYTES) {
        upload_size = UPLOAD_MAX_BYTES;
    }
    rewind(payload);

    char url[512];
    snprintf(url, sizeof(url), "%s/speedtest/upload.php", server->host);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: curl/7.68.0");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread);
    curl_easy_setopt(curl, CURLOPT_READDATA, payload);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, upload_size);
    //curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, upload_size);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sink_callback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    printf("Siunciamas payload serveriui: %s || ", server->host);
    fflush(stdout);
    CURLcode ret = curl_easy_perform(curl);

    if(ret == CURLE_OK)
    {
        long http_code = 0;
        curl_off_t bytes = 0;
        double total_time = 0.0;
        curl_off_t bps = 0;

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &bytes);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &bps);
        if(http_code == 200 && bytes > 0)
        {
            double Mbps = (double)bps * 8.0 / 1000000.0;
            server->up = Mbps;
            printf("OK nusiusta baitu: %" CURL_FORMAT_CURL_OFF_T"\n", bytes);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return Mbps;
        }
        else
        {
            server->up = 0;
            printf("serveris grazino: http: %ld, nusiusta baitu: %" CURL_FORMAT_CURL_OFF_T " (bandyta siusti: %" CURL_FORMAT_CURL_OFF_T ")\n", http_code, bytes, upload_size);
        }
    }
    else
    {
        fprintf(stdout, "Klaida: %s\n", curl_easy_strerror(ret));
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return -1;
}

int find_user_loc(char **country){
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        fprintf(stderr, "Klaida inicializuojant CURL\n");
        exit(254);
    }

    char response_buf[4096];
    response_buf[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/");//Siunciama uzklausa
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, user_loc_callback); //Kur bus siunciami duomenys
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_buf); // Nustatomas buferis, kuris bus paduotas i callback

    CURLcode curl_ret = curl_easy_perform(curl);

    if(curl_ret != CURLE_OK)
    {
        fprintf(stderr, "Klaida: %s", curl_easy_strerror(curl_ret));
        exit(254);
    }

    cJSON *response = cJSON_Parse(response_buf);
    *country = strdup(cJSON_GetObjectItem(response, "country")->valuestring);

    curl_easy_cleanup(curl);


    return 0;
}

int open_json(char *file, cJSON **root)
{
    FILE *f = fopen(file, "r");
    if (!f) {
        perror("Klaida atidarant .json faila");
        exit(255);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);//Surandame .json failo dydi

    char buf[size];
    fread(buf, 1, size, f);//Nuskaitom .json faila i bufferi
    fclose(f);

    *root = cJSON_Parse(buf);
    return cJSON_GetArraySize(*root);
}

static size_t sink_callback(void *contents, size_t size, size_t nmemb, void *bufptr)
{
    contents;
    bufptr;
    return size*nmemb;
}

static size_t user_loc_callback(void *contents, size_t size, size_t nmemb, void *bufptr){
    size_t realsize = size * nmemb;
    char *buf = (char *)bufptr;
    strncat(buf, (char *)contents, realsize);
    return realsize;
}