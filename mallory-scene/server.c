#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>

#define SERVER_PORT 8080

void error(const char *message) {
    perror(message);
    exit(1);
}

// Modular exponentiation function (a^b mod p)
uint64_t mod_exp(uint64_t a, uint64_t b, uint64_t p) {
    uint64_t result = 1;
    a = a % p;

    while (b > 0) {
        if (b & 1)
            result = (result * a) % p;

        b = b >> 1;
        a = (a * a) % p;
    }

    return result;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        error("Failed to create socket");

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("Failed to bind socket");

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0)
        error("Failed to listen");

    printf("Bob/Server listening on port %d\n", SERVER_PORT);

    // Accept a client connection
    client_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
        error("Failed to accept connection");

    printf("Client connected\n");

    // Diffie-Hellman key exchange
    uint64_t prime = 38651;    // Shared prime
    uint64_t base = 11161;     // Shared base
    uint64_t server_priv_key = 971;  //Bob private key (<prime)

    // Receive client's public key
    uint64_t client_pub_key;
    if (recv(client_fd, &client_pub_key, sizeof(client_pub_key), 0) == -1)
        error("Failed to receive client's public key");

    printf("Received client's public key: %llu\n", client_pub_key);

    // Calculate server's public key
    uint64_t server_pub_key = mod_exp(base, server_priv_key, prime);

    // Send server's public key to the client
    if (send(client_fd, &server_pub_key, sizeof(server_pub_key), 0) == -1)
        error("Failed to send server's public key");

    printf("Sent server's public key: %llu\n", server_pub_key);

    // Calculate common secret
    uint64_t shared_secret = mod_exp(client_pub_key, server_priv_key, prime);

    printf("Computed shared secret: %llu\n", shared_secret);

    // Clean up
    close(client_fd);
    close(server_fd);

    return 0;
}
