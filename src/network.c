#include <curl/curl.h>
#include "network.h"

// Callback function to write downloaded data
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

void download_data(const char *url, const char *file_name) {
    CURL *curl_handle;
    FILE *pagefile;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        pagefile = fopen(file_name, "wb");
        if (pagefile) {
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
            CURLcode res = curl_easy_perform(curl_handle);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            fclose(pagefile);
        }

        curl_easy_cleanup(curl_handle);
    }

    curl_global_cleanup();
}
