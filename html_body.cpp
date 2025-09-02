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

// Callback function to handle the response body data.
// It is called by libcurl as data is received.
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    // Append the received data to the string buffer
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }

    // Initialize a simple logger for the tool
    Logger logger;
    std::string response_body;

    // Global libcurl initialization
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL* curl = curl_easy_init();
    if (curl) {
        // Set the URL from the command line argument
        curl_easy_setopt(curl, CURLOPT_URL, argv[1]);

        // Tell libcurl where to send the received data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        // Pass our string buffer to the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            logger.log("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + "\n");
        } else {
            // Log the full body content if the request was successful
            logger.log("--- Full Body Content ---\n");
            logger.log(response_body);
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Global libcurl cleanup
    curl_global_cleanup();

    return 0;
}

