#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/stat.h>

using namespace std;

class Client {
public:
    Client(const char* serverIp, int port) : serverIp(serverIp), port(port) {}

    const char* nameClient;

    bool connectToServer() {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Error creating socket");
            return false;
        }

        sockaddr_in serverAddr;
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
        if (strcmp(message, "Null") == 0) {
            string userInput;
            cout << "Enter a command: ";
            getline(cin, userInput);

            char command[1024], filename[1024];
            if (sscanf(userInput.c_str(), "%s %s", command, filename) == 2) {
                if (strcmp(command, "PUT") == 0) {
                    sendFile(command, filename);
                }
                else if (strcmp(command, "GET") == 0) {
                    send(clientSocket, userInput.c_str(), userInput.size(), 0);
                    receiveFileFromServer(filename);
                }
                else if (strcmp(command, "CREATE_ROOM") == 0) {
                    send(clientSocket, userInput.c_str(), userInput.size(), 0);
                }
                else {
                    cout << "Invalid command" << endl;
                    sendData("Null");
                }
            }
            else {
                cout << "Invalid command" << endl;
                sendData("Null");
            }
        } else {
            send(clientSocket, message, strlen(message), 0);
        }

    }

    void receiveData() {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            cout << "Received from server:\n" << buffer << endl;
        }
    }

    void receiveFileFromServer(const string& filename) {
        ofstream file(filepath + nameClient + "/" + filename, ios::binary);

        if (!file.is_open()) {
            cout << "Error opening file for saving: " << filename << endl;
            return;
        }

        streamsize fileSize;
        recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

        const int bufferSize = 1024;
        char buffer[bufferSize];
        while (fileSize > 0) {
            ssize_t bytesRead = recv(clientSocket, buffer, min(fileSize, static_cast<streamsize>(bufferSize)), 0);
            if (bytesRead > 0) {
                file.write(buffer, bytesRead);
                fileSize -= bytesRead;
            } else {
                perror("Error receiving file data");
                break;
            }
        }

        file.close();

        cout << "File received successfully: " << filename << endl;
    }

    void sendFile(const string& command, const string& filename) {
        string data = command + " " + filename;
        send(clientSocket, data.c_str(), data.size(), 0);
        cout << (filepath + nameClient + "/" + filename) << endl;
        ifstream file(filepath + nameClient + "/" + filename, ios::binary | ios::ate);

        if (!file.is_open()) {
            cout << "Error opening file: " << filename << endl;
            return;
        }

        streamsize fileSize = file.tellg();
        file.seekg(0, ios::beg);

        send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);

        const int bufferSize = 1024;
        char buffer[bufferSize];
        while (!file.eof()) {
            file.read(buffer, bufferSize);
            send(clientSocket, buffer, file.gcount(), 0);
        }

        file.close();

        cout << "File sent successfully: " << filename << endl;
    }

    void createClientDirectory(const string& directory) {
        struct stat st;
        if (stat(directory.c_str(), &st) != 0) {
            if (mkdir(directory.c_str(), 0777) != 0) {
                perror("Error creating directory");
                return;
            }
        }
    }
private:
    int clientSocket;
    const char* serverIp;
    int port;
    string filepath = "/Users/zakerden1234/Desktop/Client-Server-Concepts/client/cmake-build-debug/client-storage/";
};

int main() {
    int port = 12345;
    const char* serverIp = "127.0.0.1";

    Client client(serverIp, port);

    if (client.connectToServer()) {
        string name;
        cout << "Enter your name:";
        getline(cin, name);
        client.sendData(name.c_str());
        client.nameClient = name.c_str();
        client.createClientDirectory("/Users/zakerden1234/Desktop/Client-Server-Concepts/client/cmake-build-debug/client-storage/" + name);

        while (true) {
            client.receiveData();
            client.sendData("Null");
        }
    }
    return 0;
}
