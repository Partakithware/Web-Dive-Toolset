#include <iostream>
#include <string>
#include <curl/curl.h>

// A simple Logger class for this standalone tool
class Logger {
public:
    void log(const std::string& message) {
        std::cout << message;
    }
};

// Callback function to handle the header data.
// It is called by libcurl for each header line.
size_t headerCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total = size * nitems;
    Logger* logger = static_cast<Logger*>(userdata);
    if (logger) {
        // Log the header line directly
        logger->log(std::string(buffer, total));
    }
    return total;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }

    // Initialize a simple logger for the tool
    Logger logger;

    // Global libcurl initialization
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL* curl = curl_easy_init();
    if (curl) {
        // Set the URL from the command line argument
        curl_easy_setopt(curl, CURLOPT_URL, argv[1]);

        // Tell libcurl to write the headers to our custom callback function
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
        // Pass our logger instance to the callback function
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &logger);

        // We are only interested in the headers, so tell libcurl not to download the body
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            logger.log("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + "\n");
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Global libcurl cleanup
    curl_global_cleanup();

    return 0;
}

