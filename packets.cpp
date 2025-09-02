#include <iostream>
#include <string>
#include <curl/curl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <chrono>
#include <iomanip>

class Logger {
public:
    void log(const std::string& msg) {
        std::cout << msg;
    }
};

Logger global_logger;

// Store resolved IP address for labeling pseudo-packets
std::string resolved_ip;
int packet_counter = 0;

// Callback to capture HTTP response body
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// Function to get a timestamp string
std::string current_timestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    auto t = system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    std::ostringstream oss;
    oss << buffer << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

// Debug callback to mimic packets
int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr) {
    std::string src = "LOCALHOST:random";   // pseudo source
    std::string dst = resolved_ip + ":443"; // pseudo destination

    auto ts = current_timestamp();
    packet_counter++;

    switch(type) {
        case CURLINFO_TEXT:
            global_logger.log("[" + ts + "] [INFO] " + std::string(data, size));
            break;
        case CURLINFO_HEADER_OUT:
            global_logger.log("[" + ts + "] [PACKET " + std::to_string(packet_counter) + "] OUT " + src + " -> " + dst + " | HEADER | " + std::to_string(size) + " bytes\n");
            global_logger.log(std::string(data, size) + "\n");
            break;
        case CURLINFO_HEADER_IN:
            global_logger.log("[" + ts + "] [PACKET " + std::to_string(packet_counter) + "] IN " + dst + " -> " + src + " | HEADER | " + std::to_string(size) + " bytes\n");
            global_logger.log(std::string(data, size) + "\n");
            break;
        case CURLINFO_DATA_OUT:
            global_logger.log("[" + ts + "] [PACKET " + std::to_string(packet_counter) + "] OUT " + src + " -> " + dst + " | DATA | " + std::to_string(size) + " bytes\n");
            break;
        case CURLINFO_DATA_IN:
            global_logger.log("[" + ts + "] [PACKET " + std::to_string(packet_counter) + "] IN " + dst + " -> " + src + " | DATA | " + std::to_string(size) + " bytes\n");
            break;
        default:
            break;
    }
    return 0;
}

// Resolve host to IPv4 address
bool resolve_host(const std::string& host, std::string& out_ip) {
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(host.c_str(), "443", &hints, &res) != 0) {
        return false;
    }

    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(res->ai_addr);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));
    out_ip = ip;

    freeaddrinfo(res);
    return true;
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>\n";
        return 1;
    }

    std::string url = argv[1];
    global_logger.log("Performing HTTPS request to: " + url + "\n");

    // Extract hostname for pseudo IP labeling
    std::string host = url.substr(url.find("://") + 3);
    size_t slash_pos = host.find('/');
    if(slash_pos != std::string::npos) host = host.substr(0, slash_pos);

    if(!resolve_host(host, resolved_ip)) {
        std::cerr << "Failed to resolve host: " << host << "\n";
        return 1;
    }

    global_logger.log("Resolved " + host + " to " + resolved_ip + "\n");

    CURL* curl = curl_easy_init();
    if(curl) {
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Enable verbose mode and debug callback
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            global_logger.log("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + "\n");
        }

        curl_easy_cleanup(curl);
    }

    global_logger.log("Request complete.\n");
    return 0;
}
