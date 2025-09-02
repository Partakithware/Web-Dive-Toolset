#include <iostream>
#include <string>
#include <curl/curl.h>
#include <string.h>
#include <algorithm> // For std::transform

// Callback function to process incoming headers
// This function is called for every header received from the server.
size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total_size = size * nitems;
    std::string header_line(buffer, total_size);

    // Create a lowercase version of the header line for a case-insensitive comparison.
    std::string lower_header = header_line;
    std::transform(lower_header.begin(), lower_header.end(), lower_header.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    // Check if the lowercase header starts with "set-cookie:"
    if (lower_header.find("set-cookie:") == 0) {
        // Print the original header line, which contains the cookie information
        std::cout << "[Cookie] " << header_line;
    }

    return total_size;
}

// Dummy write callback to discard the response body
// We are only interested in the headers for this tool.
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

int main(int argc, char* argv[]) {
    // Check if a URL was provided as a command-line argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>\n";
        return 1;
    }

    std::string url = argv[1];
    std::cout << "Performing request to: " << url << "\n\n";

    // Initialize the curl session
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl.\n";
        return 1;
    }

    // Set the URL to fetch
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set the header callback function to intercept and print cookies
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    
    // Set a dummy write callback to discard the response body, as we don't need it.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK) {
        std::cerr << "\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
    }

    // Clean up the curl session
    curl_easy_cleanup(curl);

    std::cout << "\nRequest complete.\n";

    return 0;
}
