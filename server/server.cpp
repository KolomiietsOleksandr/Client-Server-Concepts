#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

class Server {
public:
    Server(int port) : port(port) {}

    void start() {
        if (setupServer() && bindServer() && listenForConnections()) {
            std::cout << "Server listening on port " << port << std::endl;
            acceptConnections();
        }
    }

private:
    int serverSocket;
    int port;

    bool setupServer() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error creating socket");
            return false;
        }
        return true;
    }

    bool bindServer() {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            perror("Bind failed");
            close(serverSocket);
            return false;
        }
        return true;
    }

    bool listenForConnections() {
        if (listen(serverSocket, SOMAXCONN) == -1) {
            perror("Listen failed");
            close(serverSocket);
            return false;
        }
        return true;
    }

    void acceptConnections() {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
        if (clientSocket == -1) {
            perror("Accept failed");
            close(serverSocket);
            return;
        }

        std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

        handleClient(clientSocket);
    }

    void handleClient(int clientSocket) {
        char buffer[1024];
        memset(buffer, 0, 1024);

        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received data: " << buffer << std::endl;

            const char* response = "List of commands:\n 1. Get file <filename>\n 2. Get list of files\n 3. Put file <filename>\n 4. Delete <filename>\n 5. Info <filename>";
            send(clientSocket, response, strlen(response), 0);
        }

        while (true) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                std::cout << "Received data: " << buffer << std::endl;

                const char* response = "Received your message";
                send(clientSocket, response, strlen(response), 0);
            }
        }
    }
};

int main() {
    int port = 12345;
    Server server(port);
    server.start();

    return 0;
}
