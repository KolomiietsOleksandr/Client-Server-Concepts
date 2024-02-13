#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sstream>
#include <ctime>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <vector>

using namespace std;

struct Room {
    vector<int> clients;
};

class Server {
public:
    Server(int port) : port(port) {}

    void start() {
        if (setupServer() && bindServer() && listenForConnections()) {
            cout << "Server listening on port " << port << endl;
            while (true) {
                acceptConnections();
            }
        }
    }

private:
    int serverSocket;
    int port;
    string filepath = "/Users/zakerden1234/Desktop/Client-Server-Concepts/server/cmake-build-debug/server-storage/";
    unordered_map<int, string> clientDirectories;
    unordered_map<int, bool> clientRoomStatus;
    mutex clientDirectoriesMutex;
    mutex mutexCout;
    vector<string> roomsVector;
    unordered_map<string, Room> rooms;

    bool setupServer() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error creating socket");
            return false;
        }
        return true;
    }

    bool bindServer() {
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (::bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) != 0) {
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
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
        if (clientSocket == -1) {
            perror("Accept failed");
            close(serverSocket);
            return;
        }
        mutexCout.lock();
        cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
        mutexCout.unlock();
        thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }

    void handleClient(int clientSocket) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            string clientName = buffer;
            mutexCout.lock();
            cout << "Received name: " << clientName << endl;
            mutexCout.unlock();

            string clientDirectory = filepath + clientName + "/";
            lock_guard<mutex> lock(clientDirectoriesMutex);
            if (!createClientDirectory(clientDirectory)) {
                cerr << "Error creating directory for client" << endl;
                close(clientSocket);
                return;
            }

            clientDirectories[clientSocket] = clientDirectory;
            clientRoomStatus[clientSocket] = false;

            string welcomeMessage = "Hello, " + clientName + "!\n";
            welcomeMessage += "Available rooms:\n";
            for (const string& room : roomsVector) {
                welcomeMessage += " - " + room + "\n";
            }
            welcomeMessage += "Type 'CREATE_ROOM <roomname>' to create a new room or 'JOIN_ROOM <roomname>' to join an existing room.";

            send(clientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);
        }

        while (true) {
            memset(buffer, 0, sizeof(buffer));
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                mutexCout.lock();
                cout << "Received data: " << buffer << endl;
                mutexCout.unlock();
                char command[1024], value[1024];
                if (sscanf(buffer, "%s %s", command, value) == 2) {
                    mutexCout.lock();
                    cout << "Command: " << command << endl;
                    cout << "Value: " << value << endl;
                    mutexCout.unlock();
                    if (strcmp(command, "PUT") == 0) {
                        saveFile(clientSocket, value);
                    } else if (strcmp(command, "GET") == 0) {
                        sendFile(clientSocket, value);
                    } else if (strcmp(command, "CREATE_ROOM") == 0 && !clientRoomStatus[clientSocket]) {
                        createRoom(clientSocket, value);
                    } else if (strcmp(command, "JOIN_ROOM") == 0 && !clientRoomStatus[clientSocket]) {
                        joinRoom(clientSocket, value);
                    } else if (strcmp(command, "/m") == 0 && clientRoomStatus[clientSocket]) {
                        sendMessageToRoom(clientSocket, value);
                    }else {
                        send(clientSocket, "Invalid command", sizeof("Invalid command"), 0);
                    }
                }
                else{
                    if (strcmp(buffer, "LEAVE_ROOM") == 0 && clientRoomStatus[clientSocket]) {
                        leaveRoom(clientSocket);
                    } else if (strcmp(buffer, "LIST_ROOMS") == 0 && !clientRoomStatus[clientSocket]) {
                        listRooms(clientSocket);
                    }
                }
            }
        }
    }

    void sendMessageToRoom(int senderSocket, const char* message) {
        lock_guard<mutex> lock(clientDirectoriesMutex);
        for (auto& room : rooms) {
            auto& clients = room.second.clients;
            auto it = find(clients.begin(), clients.end(), senderSocket);
            if (it != clients.end()) {
                for (int client : clients) {
                    if (client != senderSocket) {
                        send(client, message, strlen(message), 0);
                    }
                }
                break;
            }
        }
    }

    void joinRoom(int clientSocket, const char* roomname) {
        lock_guard<mutex> lock(clientDirectoriesMutex);
        auto it = rooms.find(roomname);
        if (it != rooms.end()) {
            it->second.clients.push_back(clientSocket);
            clientRoomStatus[clientSocket] = true;
            send(clientSocket, "Joined room successfully", sizeof("Joined room successfully"), 0);
        } else {
            send(clientSocket, "Room does not exist", sizeof("Room does not exist"), 0);
        }
    }

    void leaveRoom(int clientSocket) {
        lock_guard<mutex> lock(clientDirectoriesMutex);
        for (auto& room : rooms) {
            auto& clients = room.second.clients;
            auto it = find(clients.begin(), clients.end(), clientSocket);
            if (it != clients.end()) {
                clients.erase(it);
                clientRoomStatus[clientSocket] = false;
                break;
            }
        }
        send(clientSocket, "Left room successfully", sizeof("Left room successfully"), 0);
    }

    void listRooms(int clientSocket) {
        string roomList = "Available rooms:\n";
        for (const string& room : roomsVector) {
            roomList += " - " + room + "\n";
        }
        send(clientSocket, roomList.c_str(), roomList.size(), 0);
    }

    bool createClientDirectory(const string& directory) {
        struct stat st;
        if (stat(directory.c_str(), &st) != 0) {
            if (mkdir(directory.c_str(), 0777) != 0) {
                perror("Error creating directory");
                return false;
            }
        }
        return true;
    }

    void saveFile(int clientSocket, const char* filename) {
        string clientDirectory = clientDirectories[clientSocket];
        ofstream file(clientDirectory + filename, ios::binary);
        if (!file.is_open()) {
            perror("Error opening file for saving");
            send(clientSocket, "Error saving file", strlen("Error saving file"), 0);
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
        mutexCout.lock();
        cout << "File saved successfully: " << filename << endl;
        mutexCout.unlock();
        send(clientSocket, "File saved successfully", strlen("File saved successfully"), 0);
    }

    void sendFile(int clientSocket, const char* filename) {
        string clientDirectory = clientDirectories[clientSocket];
        ifstream file(clientDirectory + filename, ios::binary);
        if (!file.is_open()) {
            perror("Error opening file for sending");
            send(clientSocket, "Error opening file for sending", strlen("Error opening file for sending"), 0);
            return;
        }

        file.seekg(0, ios::end);
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
        mutexCout.lock();
        cout << "File sent successfully: " << filename << endl;
        mutexCout.unlock();
        send(clientSocket, "Successfully sent", sizeof("Successfully sent"), 0);
    }

    void createRoom(int clientSocket, const char* roomname) {
        lock_guard<mutex> lock(clientDirectoriesMutex);
        roomsVector.push_back(roomname);
        rooms[roomname];
        send(clientSocket, "Room created successfully", sizeof("Room created successfully"), 0);
    }
};

int main() {
    int port = 12345;
    Server server(port);
    server.start();

    return 0;
}
