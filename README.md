Web-Dive-Toolset
===
    DNS Resolution: Translating a hostname into an IP address.

    Packet Logging: Simulating and displaying network traffic at a basic level.

    Header Inspection: Fetching and displaying HTTP headers from a web server response.

    Cookie Management: Capturing and reporting Set-Cookie headers.

    Redirection Analysis: Tracing and reporting the redirection chain of a URL.

    Content Retrieval: Downloading and displaying the main body of an HTML page.

    TLS/SSL Diagnostics: Providing detailed information about the TLS session and server certificates, including the certificate chain.

// GUI - Run them all in one place no CLI - (make sure each binary has been built and run this in the same directory) [It is not standalone]
``` g++ gui.cpp -o gui `pkg-config --cflags --libs gtk+-3.0` ```

// CertChain

```g++ certchain.cpp -o certchain -lcurl -lssl -lcrypto```

// DNS

```g++ dns.cpp -o dns```

// Header

```g++ header.cpp -o header -lcurl```

// HTML Body - Full

```g++ html_body.cpp -o html_body -lcurl```

// Packets

```g++ packets.cpp -o packets -lcurl -lpcap -lresolv```

// Cookies 

```g++ cookies.cpp -o cookies -lcurl```

// TLS (Ehh)

```g++ tls.cpp -o tls -lcurl -lssl -lcrypto```

Well simply put I was just messing around. Make use of them or don't.

To use each its quite simple

```./toolname url-you-want-to-get-data-from```

Then it will output the data to console. Enjoy. (Simple Approach to Each libcurl)
Test them yourself to see what data each will provide.
