#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h"
#include <stdlib.h>
#include "tests.h"
#include <string.h>

#define UPLOAD_MAX_BYTES (8 * 1024 * 1024)

int main(int argc, char **argv){
    CURLcode global_status = curl_global_init(CURL_GLOBAL_ALL);
    if(global_status != CURLE_OK){
        perror("Klaida su curl: ");
        return -1;
    }

    char *list_file = "speedtest_server_list.json";

    //Tikrinama ar norima atlikti individualu testa
    if(argc == 3){
        char *test = argv[1]; //testo pavadinimas
        char *arg1 = argv[2]; //argumentas
        char *user_country;
        find_user_loc(&user_country);

        printf("Naudotojo vietove: %s\n", user_country);
        if(strcmp(test, "location") == 0)
        {
            cJSON *root = NULL;
            int count = open_json(list_file, &root);
            struct result results[count]; // Rezultatu masyvas
            int accepted_count = filtrate_loc(count, root, arg1, results);
            free(root);

            printf("Vietove: %s    Rasta serveriu: %d\n", arg1, accepted_count);
            //Atrinktiem serveriam atliekami testai
            //===========================================================================
            //Geriausias serveris pagal parsiuntimo greiti
            printf("==== Atliekamas parsiuntimo greicio testas =====\n");
            struct result *best_download = malloc(sizeof(struct result));
            best_download->down = 0;
            best_download->host = "";
            best_download->up = 0;
            FILE *payload = NULL;
            download_speed_test_all(results, accepted_count, best_download, &payload);
            if (payload != NULL) {
                upload_speed_test(best_download, payload);
            }

            curl_global_cleanup();

            //Geriausias serveris pagal siuntimo greiti
            printf("==== Atliekamas siuntimo greicio testas =====\n");
            struct result *best_upload = malloc(sizeof(struct result));
            best_upload->down = 0;
            best_upload->host = "";
            best_upload->up = 0;
            upload_speed_test_all(results, accepted_count, best_upload, payload);
            if (payload != NULL && strcmp(best_upload->host, "") != 0) {
                download_speed_test(best_upload, &payload);
                fclose(payload);
            }

            curl_global_cleanup();

            print_result(*best_download, "Geriausias serveris pagal parsiuntimo greiti");
            print_result(*best_upload, "Geriausias serveris pagal siuntimo greiti");

            free(best_download);
            free(best_upload);

            return 0;
        }
        else if(strcmp(test, "download") == 0)
        {
            struct result server;
            server.host = arg1;
            server.down = 0;
            server.up = 0;
            //FILE *payload = NULL;
            double Mbps = download_speed_test(&server, NULL);
            if(Mbps == -1)
            {
                fprintf(stderr, "Klaida testuojant serverio greiti\n");
                /*if(payload != NULL)
                {
                    fclose(payload);
                }*/
                return -1;
            }
            /*if(payload != NULL)
            {
                fclose(payload);
            }*/
            print_result(server, "Parsiuntimo greicio rezultatai");
        }
        else if(strcmp(test, "upload") == 0)
        {
            struct result server;
            server.host = arg1;
            FILE *payload = NULL;
            printf("Parsisiunciamas payload ||");
            double down_mbps = download_speed_test(&server, &payload);
            if(down_mbps <= 0 || payload == NULL)
            {
                fprintf(stderr, "Klaida: nepavyko parsiusti payload upload testui\n");
                if (payload != NULL) {
                    fclose(payload);
                }
                return -1;
            }
            double Mbps = upload_speed_test(&server, payload);
            fclose(payload);
            if(Mbps == -1)
            {
                fprintf(stderr, "Klaida testuojant serverio greiti\n");
                return -1;
            }
            print_result(server, "Siuntimo greicio rezultatai");
        }
        return 0;
    }

    //Naudotojo ip informacijos nuskaitymas per API
    //======================================================================
    char *country;
    find_user_loc(&country);

    //Atidaromas .json failas ==================================================
    cJSON *root = NULL;
    int count = open_json(list_file, &root);
    //======================================================================

    struct result results[count]; // Rezultatu masyvas

    //Filtravimas pagal vietove
    //=========================================================================
    int accepted_count = filtrate_loc(count, root, country, results);
    printf("Vietove: %s    Rasta serveriu: %d\n", country, accepted_count);

    free(root);

    //Atrinktiem serveriam atliekami testai
    //===========================================================================
    //Geriausias serveris pagal parsiuntimo greiti
    printf("==== Atliekamas parsiuntimo greicio testas =====\n");
    struct result *best_download = malloc(sizeof(struct result));
    best_download->down = 0;
    best_download->host = "";
    best_download->up = 0;
    FILE *payload = NULL;
    download_speed_test_all(results, accepted_count, best_download, &payload);
    if (payload != NULL) {
        upload_speed_test(best_download, payload);
    }

    curl_global_cleanup();

    //Geriausias serveris pagal siuntimo greiti
    printf("==== Atliekamas siuntimo greicio testas =====\n");
    struct result *best_upload = malloc(sizeof(struct result));
    best_upload->down = 0;
    best_upload->host = "";
    best_upload->up = 0;
    upload_speed_test_all(results, accepted_count, best_upload, payload);
    if (payload != NULL && strcmp(best_upload->host, "") != 0) {
        download_speed_test(best_upload, &payload);
        fclose(payload);
    }

    curl_global_cleanup();

    print_result(*best_download, "Geriausias serveris pagal parsiuntimo greiti");
    print_result(*best_upload, "Geriausias serveris pagal siuntimo greiti");

    free(best_download);
    free(best_upload);

    return 0;
}