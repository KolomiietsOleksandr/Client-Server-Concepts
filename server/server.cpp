#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>

using namespace std;

class Server {
public:
    Server(int port) : port(port) {}

    void start() {
        if (setupServer() && bindServer() && listenForConnections()) {
            cout << "Server listening on port " << port << endl;
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
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
        if (clientSocket == -1) {
            perror("Accept failed");
            close(serverSocket);
            return;
        }

        cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;

        handleClient(clientSocket);
    }

    void handleClient(int clientSocket) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            cout << "Received data: " << buffer << endl;

            const char* response = "List of commands:\n 1. Get file <filename>\n 2. Get list of files\n 3. Put file <filename>\n 4. Delete <filename>\n 5. Info <filename>";
            send(clientSocket, response, strlen(response), 0);
        }

        while (true) {
            memset(buffer, 0, sizeof(buffer));
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                cout << "Received data: " << buffer << endl;

                char command[1024], filename[1024];
                if (sscanf(buffer, "%s %s", command, filename) == 2) {
                    cout << "Command: " << command << endl;
                    cout << "Filename: " << filename << endl;

                    if (strcmp(command, "PUT") == 0) {
                        saveFile(clientSocket, filename);
                    } else if (strcmp(command, "GET") == 0) {
                        sendFile(clientSocket, filename);
                    } else if (strcmp(command, "DELETE") == 0) {
                        deleteFile(clientSocket, filename);
                    }
                }

                if (strcmp(command, "LIST") == 0) {
                    listFiles(clientSocket);
                }
            }
        }
    }

    void saveFile(int clientSocket, const char* filename) {
        string filepath = "/Users/zakerden1234/Desktop/Client-Server-Concepts/server/cmake-build-debug/server-storage/";
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

        cout << "File saved successfully: " << filename << endl;
        send(clientSocket, "File saved successfully", strlen("File saved successfully"), 0);
    }

    void sendFile(int clientSocket, const char* filename) {
        string filepath = "/Users/zakerden1234/Desktop/Client-Server-Concepts/server/cmake-build-debug/server-storage/";
        ifstream file(filepath + filename, ios::binary);

        if (!file.is_open()) {
            cout << "Error opening file: " << filename << endl;
            send(clientSocket, "Error opening file", strlen("Error opening file"), 0);
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

    void deleteFile(int clientSocket, const char* filename) {
        string filepath = "/Users/zakerden1234/Desktop/Client-Server-Concepts/server/cmake-build-debug/server-storage/";

        if (remove((filepath + filename).c_str()) != 0) {
            perror("Error deleting file");
            send(clientSocket, "Error deleting file", strlen("Error deleting file"), 0);
        } else {
            cout << "File deleted successfully: " << filename << endl;
            send(clientSocket, "File deleted successfully", strlen("File deleted successfully"), 0);
        }
    }

    void listFiles(int clientSocket) {
        string filepath = "/Users/zakerden1234/Desktop/Client-Server-Concepts/server/cmake-build-debug/server-storage/";
        DIR* dir;
        struct dirent* ent;

        if ((dir = opendir(filepath.c_str())) != NULL) {
            string filesList = "Files and Directories in server-storage:\n";
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG || ent->d_type == DT_DIR) {
                    filesList += ent->d_name;
                    filesList += (ent->d_type == DT_DIR ? " [Directory]" : " [File]");
                    filesList += "\n";
                }
            }
            closedir(dir);

            send(clientSocket, filesList.c_str(), filesList.size(), 0);
        } else {
            perror("Error opening directory");
            send(clientSocket, "Error listing files", strlen("Error listing files"), 0);
        }
    }
};

int main() {
    int port = 12345;
    Server server(port);
    server.start();

    return 0;
}
