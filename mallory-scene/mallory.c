#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h> // For inet_addr
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT_DEST 8080
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
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    uint64_t intercepted;
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

    printf("Interceptor listening on port %d\n", SERVER_PORT);

    // Accept a client connection
    socklen_t client_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
        error("Failed to accept connection");

    printf("Client connected\n");

    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        error("Failed to create server socket");

    // Set server address
    struct sockaddr_in server_socket_addr;
    server_socket_addr.sin_family = AF_INET;
    server_socket_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_socket_addr.sin_port = htons(SERVER_PORT_DEST);

    // Connect to the actual server
    if (connect(server_socket, (struct sockaddr *)&server_socket_addr, sizeof(server_socket_addr)) < 0)
        error("Failed to connect to server");

    printf("Connected to server\n");
    
    // Diffie-Hellman key exchange
    uint64_t prime = 38651;    // Shared prime
    uint64_t base = 11161;     // Shared base

    // Generate Mallory's private key
    uint64_t mallory_priv_key = 499; //Mallory's private key (<prime)
    
    // Calculate Mallory's public key
    uint64_t mallory_pub_key = mod_exp(base, mallory_priv_key, prime);

    // Var to store each public key
    uint64_t client_pub_key, server_pub_key;

    while (1) {
        // Receive data from client
        ssize_t bytes_received = recv(client_fd, &intercepted, sizeof(intercepted), 0);
        if (bytes_received <= 0)
            break;
        client_pub_key = intercepted;
        printf("Client (Alice) recv : %llu\n", client_pub_key);

        // Modify the intercepted data
        // Forward modified data to the server
        if (send(server_socket, &mallory_pub_key, sizeof(mallory_pub_key), 0) == -1)
            break;

        // Receive response from server
        ssize_t bytes_response = recv(server_socket, &intercepted, sizeof(intercepted), 0);
        if (bytes_response <= 0)
            break;
        
        server_pub_key = intercepted;
        printf("Server (Bob) recv : %llu\n", server_pub_key);

        // Modify the intercepted response
        // Forward modified response to the client
        if (send(client_fd, &mallory_pub_key, sizeof(mallory_pub_key), 0) == -1)
            break;
    }
    
    // Calculate their secret keys
    uint64_t client_secret_key = mod_exp(client_pub_key, mallory_priv_key, prime);
    uint64_t server_secret_key = mod_exp(server_pub_key, mallory_priv_key, prime);
    
    printf("Client Secret Key : %llu\n", client_secret_key);
    printf("Server Secret Key : %llu\n", server_secret_key);

    // Clean up
    close(client_fd);
    close(server_fd);

    return 0;
}
