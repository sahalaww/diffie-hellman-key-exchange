#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

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
    int client_fd;
    struct sockaddr_in server_addr;

    // Create client socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        error("Failed to create socket");

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("Failed to connect to server");

    printf("Alice connected to Bob\n");

    // Diffie-Hellman key exchange
    uint64_t prime = 38651;    // Shared prime
    uint64_t base = 11161;     // Shared base

    // Generate client's private key
    uint64_t client_priv_key = 577; //Alice private key (<prime)

    // Calculate client's public key
    uint64_t client_pub_key = mod_exp(base, client_priv_key, prime);

    // Send client's public key to the server
    if (send(client_fd, &client_pub_key, sizeof(client_pub_key), 0) == -1)
        error("Failed to send client's public key");

    printf("Sent client's public key: %llu\n", client_pub_key);

    // Receive server's public key
    uint64_t server_pub_key;
    if (recv(client_fd, &server_pub_key, sizeof(server_pub_key), 0) == -1)
        error("Failed to receive server's public key");

    printf("Received server's public key: %llu\n", server_pub_key);

    // Calculate common secret
    uint64_t shared_secret = mod_exp(server_pub_key, client_priv_key, prime);

    printf("Computed shared secret: %llu\n", shared_secret);

    // Clean up
    close(client_fd);

    return 0;
}
