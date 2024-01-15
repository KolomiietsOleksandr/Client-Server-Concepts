#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

class Client {
public:
    Client(const char* serverIp, int port) : serverIp(serverIp), port(port) {}

    bool connectToServer() {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Error creating socket");
            return false;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr));

        if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            perror("Connect failed");
            close(clientSocket);
            return false;
        }

        return true;
    }

    void sendData(const char* message) {
        send(clientSocket, message, strlen(message), 0);
    }

    void receiveData() {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received from server:\n" << buffer << std::endl;
        }
    }

    void closeConnection() {
        close(clientSocket);
    }

private:
    int clientSocket;
    const char* serverIp;
    int port;
};

int main() {
    // Client configuration
    int port = 12345;
    const char* serverIp = "127.0.0.1";

    Client client(serverIp, port);

    if (client.connectToServer()) {
        const char* message = "User connected";
        client.sendData(message);

        while (true) {
            client.receiveData();
        }
        // client.closeConnection();
    }

    return 0;
}
