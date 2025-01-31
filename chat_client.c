#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h> 

#define PORT 49153
#define SERVER "vmwardrobe.westmont.edu"

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set up server details
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    struct hostent *host = gethostbyname(SERVER);
    if (host == NULL) {
        perror("Failed to resolve hostname");
        return 1;
    }
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);


    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to server. Enter your username: ");
    fgets(buffer, sizeof(buffer), stdin);
    send(sock, buffer, strlen(buffer), 0);

    // Message loop
    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        printf("Server: %s", buffer);
    }

    close(sock);
    return 0;
}
