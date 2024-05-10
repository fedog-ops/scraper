#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

size_t got_data(char *buffer, size_t itemsize, size_t nitems, void* ignorethis){
    size_t bytes = itemsize * nitems;
    int linenumber = 1;
    printf("New chunk (%zu bytes)\n", bytes);
    for (int i = 0; i< bytes; i++){
        printf("%c", buffer[i]);
        if (buffer[i] == '\n'){
            linenumber++;
            printf("%d:\t", linenumber);
        }
    }
    return bytes;
}
//return the number of bytes the function processed


int main () {
    CURL *curl = curl_easy_init();

    if(!curl){
        fprintf(stderr, "init failed\n");
        return EXIT_FAILURE;
    }

//set options
    curl_easy_setopt(curl, CURLOPT_URL, "https://scrapeme.live/shop/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, got_data); //pointer to a function

    //perform out an action
    CURLcode result = curl_easy_perform(curl);
    if(result != CURLE_OK){
        fprintf(stderr, "download problem: %s \n", curl_easy_strerror(result));
    }

    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;

  
}
