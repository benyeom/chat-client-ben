#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <termios.h>

#define PORT 49153
#define SERVER "vmwardrobe.westmont.edu"

int set_socket_non_blocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL) failed");
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) == -1) {
        perror("fcntl(F_SETFL) failed");
        return -1;
    }
    return 0;
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];
    fd_set read_fds;
    struct timeval timeout;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket to non-blocking mode
    if (set_socket_non_blocking(sock) < 0) {
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
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);   // Check for user input (stdin)
        FD_SET(sock, &read_fds); // Check for incoming messages from server

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Select on sockets
        int activity = select(sock + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        if (FD_ISSET(0, &read_fds)) {
            // User input is available
            printf("> ");
            fgets(buffer, sizeof(buffer), stdin);
            send(sock, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(sock, &read_fds)) {
            // Message from server is available
            memset(buffer, 0, sizeof(buffer));
            int recv_size = recv(sock, buffer, sizeof(buffer), 0);
            if (recv_size > 0) {
                printf("\rServer: %s", buffer);
                // Reprint the user's current input after the server's message
                printf("> ");
                fflush(stdout); // Ensure the prompt is shown correctly
            }
        }
    }

    close(sock);
    return 0;
}

