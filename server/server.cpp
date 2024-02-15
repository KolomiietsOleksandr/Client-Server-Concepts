#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <algorithm>
#include <unistd.h>

using namespace std;

struct Room {
    vector<int> clients;
    int fileProcessedCount = 0;
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
    unordered_map<int, string> clientNames;
    unordered_map<int, bool> clientRoomStatus;
    mutex clientDirectoriesMutex;
    mutex mutexCout;
    vector<string> roomsVector;
    unordered_map<string, Room> rooms;
    bool waiting = false;
    string waitingFilename;

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

            clientNames[clientSocket] = clientName;

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
                cout << "Received data from " << clientNames[clientSocket] << ": " << buffer << endl;
                mutexCout.unlock();

                string command(buffer);
                size_t pos = command.find(' ');
                string cmd = command.substr(0, pos);

                string value = (pos != string::npos) ? command.substr(pos + 1) : "";
                if (!cmd.empty()) {
                    mutexCout.lock();
                    cout << "Command: " << cmd << endl;
                    cout << "Value: " << value << endl;
                    mutexCout.unlock();
                    if (cmd == "/y" && clientRoomStatus[clientSocket]) {
                        sendFileToClient(clientSocket, waitingFilename.c_str());
                        markFileProcessed(clientSocket);
                    } else if (cmd == "/n" && clientRoomStatus[clientSocket]) {
                        cout << "Client " << clientNames[clientSocket] << " declined the file." << endl;
                        markFileProcessed(clientSocket);
                    } else if (cmd == "CREATE_ROOM" && !clientRoomStatus[clientSocket]) {
                        createRoom(clientSocket, value.c_str());
                    } else if (cmd == "JOIN_ROOM" && !clientRoomStatus[clientSocket]) {
                        joinRoom(clientSocket, value.c_str());
                    } else if (cmd == "/m" && clientRoomStatus[clientSocket]) {
                        sendMessageToRoom(clientSocket, value.c_str());
                    } else if (cmd == "/f" && clientRoomStatus[clientSocket]) {
                        waitingFilename = value;
                        saveFile(clientSocket, value.c_str());
                        string msg = "/f " + value;
                        cout << msg << endl;
                        sendMessageToRoom(clientSocket, msg.c_str());
                    } else if (cmd == "LEAVE_ROOM" && clientRoomStatus[clientSocket]) {
                        leaveRoom(clientSocket);
                    } else if (strcmp(buffer, "LIST_ROOMS") == 0 && !clientRoomStatus[clientSocket]) {
                        listRooms(clientSocket);
                    } else {
                        send(clientSocket, "Invalid command server", sizeof("Invalid command server"), 0);
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
                    string messageStr(message);
                    if (client != senderSocket && messageStr.find("/f") == string::npos) {
                        string clientName = clientNames[senderSocket];
                        string formattedMessage = clientName + ": " + messageStr;
                        send(client, formattedMessage.c_str(), formattedMessage.size(), 0);
                    } else if (client != senderSocket && messageStr.find("/f") == 0) {
                        string filename = messageStr.substr(3);
                        string filepath = this->filepath + filename;

                        ifstream file(filepath, ios::binary);
                        if (!file.is_open()) {
                            cerr << "Error opening file for size: " << filename << endl;
                            continue;
                        }

                        file.seekg(0, ios::end);
                        size_t fileSize = file.tellg();
                        file.close();

                        string requestMessage = "Do you want to receive the file '" + filename + "' (" + to_string(fileSize) + " bytes) from " + clientNames[senderSocket] + "? ('/y' or '/no')";
                        send(client, requestMessage.c_str(), requestMessage.size(), 0);
                    }
                }
                break;
            }
        }
    }

    void markFileProcessed(int clientSocket) {
        for (auto& room : rooms) {
            auto& clients = room.second.clients;
            auto it = find(clients.begin(), clients.end(), clientSocket);
            if (it != clients.end()) {
                room.second.fileProcessedCount++;
                if (room.second.fileProcessedCount == clients.size() -1 ) {
                    deleteFile(waitingFilename);
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
            send(clientSocket, "Joined room successfully.\nType '/m <massage>' to send text for user users.\nType 'LEAVE_ROOM' to exit.", sizeof("Joined room successfully.\nType '/m <massage>' to send text for user users.\nType 'LEAVE_ROOM' to exit."), 0);
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
        roomList += "Type 'CREATE_ROOM <roomname>' to create a new room or 'JOIN_ROOM <roomname>' to join an existing room.";
        send(clientSocket, roomList.c_str(), roomList.size(), 0);
    }

    void saveFile(int clientSocket, const char* filename) {
        ofstream file(filepath + filename, ios::binary);
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
    }

    void sendFileToClient(int clientSocket, const std::string& filename) {
        string fileTypeIndicator = "file";
        send(clientSocket, fileTypeIndicator.c_str(), fileTypeIndicator.size(), 0);

        sleep(1);

        send(clientSocket, filename.c_str(), filename.size(), 0);

        ifstream file(filepath + filename, std::ios::binary);
        if (!file.is_open()) {
            cerr << "Error opening file for sending: " << filename << std::endl;
            return;
        }
        file.seekg(0, std::ios::end);
        streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        send(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

        const int bufferSize = 1024;
        char buffer[bufferSize];
        while (!file.eof()) {
            file.read(buffer, bufferSize);
            ssize_t bytesRead = file.gcount();
            if (send(clientSocket, buffer, bytesRead, 0) == -1) {
                perror("Error sending file data to client");
                return;
            }
        }
        file.close();

        cout << "File sent successfully: " << filename << std::endl;
    }


    void createRoom(int clientSocket, const char* roomname) {
        lock_guard<mutex> lock(clientDirectoriesMutex);
        roomsVector.push_back(roomname);
        rooms[roomname];
        send(clientSocket, "Room created successfully", sizeof("Room created successfully"), 0);
    }

    void deleteFile(const std::string& filename) {
        std::string fullFilePath = filepath + filename;
        if (remove(fullFilePath.c_str()) != 0) {
            perror("Error deleting file");
        } else {
            cout << "File deleted successfully: " << filename << endl;
        }
    }
};

int main() {
    int port = 12345;
    Server server(port);
    server.start();

    return 0;
}
