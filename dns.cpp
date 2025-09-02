#include <iostream>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>

// A simple Logger class for this standalone tool
class Logger {
public:
    void log(const std::string& message) {
        std::cout << message;
    }
};

// DNSResolver class for resolving hostnames to IP addresses
class DNSResolver {
public:
    DNSResolver(Logger& logger) : logger_(logger) {}

    void resolve(const std::string& hostname) {
        logger_.log("[DNS] Resolving " + hostname + "...\n");
        addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        if (getaddrinfo(hostname.c_str(), NULL, &hints, &res) != 0) {
            logger_.log("[DNS] Resolution failed\n");
            return;
        }
        for (auto p = res; p; p = p->ai_next) {
            sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            logger_.log("[DNS] A Record: " + std::string(ip) + "\n");
        }
        freeaddrinfo(res);
    }

private:
    Logger& logger_;
};

// Utility function to extract the hostname from a URL
std::string getHostnameFromUrl(const std::string& url) {
    size_t start = url.find("//");
    if (start != std::string::npos) {
        size_t end = url.find('/', start + 2);
        return url.substr(start + 2, (end == std::string::npos ? url.size() : end) - (start + 2));
    }
    return url;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url_or_hostname>" << std::endl;
        return 1;
    }

    std::string input = argv[1];
    std::string hostname = getHostnameFromUrl(input);

    Logger logger;
    DNSResolver dnsResolver(logger);
    dnsResolver.resolve(hostname);

    return 0;
}