#include <iostream>
#include <string>
#include <curl/curl.h>
#include <algorithm> // For std::transform

// Dummy write callback to discard the response body
// We are only interested in the headers and redirect chain for this tool.
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

// Header callback function to print the headers and redirect info
size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total_size = size * nitems;
    std::string header_line(buffer, total_size);

    // Check for HTTP status line at the beginning of the header block
    if (header_line.find("HTTP/") == 0) {
        std::cout << "\n[Status] " << header_line.substr(0, header_line.find('\n'));
    }

    // Check for "Location:" header, which indicates a redirect
    std::string lower_header = header_line;
    std::transform(lower_header.begin(), lower_header.end(), lower_header.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (lower_header.find("location:") == 0) {
        std::string location_url = header_line.substr(header_line.find(":") + 1);
        // Trim leading/trailing whitespace
        location_url.erase(0, location_url.find_first_not_of(" \t\n\r\f\v"));
        std::cout << "[Redirect] -> " << location_url;
    }

    return total_size;
}

int main(int argc, char* argv[]) {
    // Check if a URL was provided as a command-line argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>\n";
        return 1;
    }

    std::string url = argv[1];
    std::cout << "Performing request to: " << url << "\n";

    // Initialize the curl session
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl.\n";
        return 1;
    }

    // Set the initial URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Enable automatic redirection following
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set header and write callback functions
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
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
