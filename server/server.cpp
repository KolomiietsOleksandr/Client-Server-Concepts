#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    // Server configuration
    int port = 12345;

    // Create a TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Bind the socket to a specific address and port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    // Accept a client connection
    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
    if (clientSocket == -1) {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

    // Receive data from the client
    char buffer[1024];
    memset(buffer, 0, 1024);

    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Received data: " << buffer << std::endl;

        // Send a response back to the client
        const char* response = "Hello, client! This is the server.";
        send(clientSocket, response, strlen(response), 0);
    }

    // Clean up
    close(clientSocket);
    close(serverSocket);

    return 0;
}