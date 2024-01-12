#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    // Client configuration
    int port = 12345;
    const char* serverIp = "127.0.0.1";

    // Create a TCP socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Connect the socket to the server's address and port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr));

    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Connect failed");
        close(clientSocket);
        return 1;
    }

    // Send data to the server
    const char* message = "Hello, server! How are you?";
    send(clientSocket, message, strlen(message), 0);

    // Receive the response from the server
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer)); // Initialize buffer with zeros
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Received from server: " << buffer << std::endl;
    }

    // Clean up
    close(clientSocket);
    return 0;
}