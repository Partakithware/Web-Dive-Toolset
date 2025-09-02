#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

// Callback to discard response body (we only care about TLS info)
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

// Helper to print a certificate's info
void print_certificate(X509* cert, int index) {
    if (!cert) return;

    char* subj = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), nullptr, 0);
    std::cout << "Certificate " << index << ":\n";
    std::cout << "  Subject: " << (subj ? subj : "N/A") << "\n";
    std::cout << "  Issuer: " << (issuer ? issuer : "N/A") << "\n";

    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int n = 0;
    if (X509_digest(cert, EVP_sha256(), md, &n)) {
        std::cout << "  SHA-256 Fingerprint: ";
        for (unsigned int i = 0; i < n; i++) {
            printf("%02X", md[i]);
            if (i + 1 < n) printf(":");
        }
        std::cout << "\n";
    }

    OPENSSL_free(subj);
    OPENSSL_free(issuer);
}

// Deep dive TLS info
void print_tls_info(SSL* ssl) {
    if (!ssl) {
        std::cerr << "No SSL session available.\n";
        return;
    }

    const SSL_CIPHER* cipher = SSL_get_current_cipher(ssl);
    std::cout << "\n[TLS Information]\n";
    std::cout << "  Protocol Version: " << SSL_get_version(ssl) << "\n";

    if (cipher) {
std::cout << "  Cipher Suite (full): " << SSL_CIPHER_get_name(cipher) << "\n";
int bits = 0;
SSL_CIPHER_get_bits(cipher, &bits);
std::cout << "  Cipher Bits: " << bits << "\n";
    }

    // ALPN Protocol (if negotiated)
    const unsigned char* proto = nullptr;
    unsigned int proto_len = 0;
    SSL_get0_alpn_selected(ssl, &proto, &proto_len);
    if (proto && proto_len > 0) {
        std::cout << "  ALPN Protocol: " << std::string((const char*)proto, proto_len) << "\n";
    }

    // Certificate chain
    STACK_OF(X509)* chain = SSL_get_peer_cert_chain(ssl);
    if (chain) {
        int n = sk_X509_num(chain);
        for (int i = 0; i < n; i++) {
            X509* cert = sk_X509_value(chain, i);
            print_certificate(cert, i + 1);
        }
    }

    // Peer leaf certificate
    X509* leaf = SSL_get_peer_certificate(ssl);
    if (leaf) {
        std::cout << "Leaf certificate info:\n";
        print_certificate(leaf, 0);
        X509_free(leaf);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <url>\n";
        return 1;
    }

    std::string url = argv[1];
    std::cout << "Connecting to: " << url << "\n";

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl.\n";
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return 1;
    }

    // Retrieve SSL* from libcurl
    // Modern curl >= 7.64 supports CURLINFO_TLS_SSL_PTR
    SSL* ssl = nullptr;
    res = curl_easy_getinfo(curl, CURLINFO_TLS_SSL_PTR, &ssl);
    if (res != CURLE_OK || !ssl) {
        std::cerr << "Failed to get SSL session info.\n";
        curl_easy_cleanup(curl);
        return 1;
    }

    print_tls_info(ssl);

    curl_easy_cleanup(curl);
    return 0;
}
