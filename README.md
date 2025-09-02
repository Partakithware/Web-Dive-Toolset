// CertChain
g++ certchain.cpp -o certchain -lcurl -lssl -lcrypto

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

./toolname url-you-want-to-get-data-from

Then it will output the data to console. Enjoy. (Simple Approach to Each libcurl)
Test them yourself to see what data each will provide.
