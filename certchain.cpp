#include <iostream>
#include <string>
#include <curl/curl.h>

void printChain(CURL* curl) {
    struct curl_certinfo* certinfo = nullptr;
    CURLcode res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &certinfo);

    if (res != CURLE_OK || !certinfo) {
        std::cout << "No SSL certificate information available." << std::endl;
        return;
    }

    std::cout << "--- Certificate Chain ---" << std::endl;

    for (int i = 0; i < certinfo->num_of_certs; ++i) {
        std::cout << "\nCertificate " << (i + 1) << ":" << std::endl;
        struct curl_slist* slist = certinfo->certinfo[i];
        while (slist) {
            std::cout << "  " << slist->data << std::endl;
            slist = slist->next;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            printChain(curl);
        } else {
            std::cerr << "curl_easy_perform() failed: " 
                      << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
