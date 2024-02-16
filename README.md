# TCP/IP Chat Server and File Transfer System
## Overview
This project implements a TCP/IP-based chat server and file transfer system. It allows clients to connect to the server, create/join chat rooms, exchange text messages, and send/receive files within the chat rooms. The server manages multiple clients concurrently, ensuring seamless communication and file transfer.

## Features
- **Server Setup:** The server is capable of setting up a socket, binding it to a port, and listening for incoming connections.
- **Client Connections:** Multiple clients can connect to the server simultaneously.
- **Chat Rooms:** Clients can create and join chat rooms, facilitating organized communication.
- **Text Messaging:** Clients can exchange text messages within chat rooms.
- **File Transfer:** Clients can send files to other clients within the same chat room.
- **Room Management:** Clients can leave chat rooms, and the server manages room membership dynamically.
- **Error Handling:** The server gracefully handles errors such as failed socket creation, binding, and file operations.

## Expected Message Format
- **Text Messages:** Text messages are sent as plain strings, with each message terminated by a newline character ('\n').
- **File Transfer Requests:** File transfer requests are sent in the format /f filename, where 'filename' is the name of the file to be transferred.
- **Confirmation Responses:** Confirmation responses for file transfers are sent as '/y' (for accepting) or '/n' (for rejecting).
- **Command Responses:** Responses to server commands (e.g., creating/joining rooms) are sent as plain strings.

## Data Transmission
**Text Messages:** Text messages are transmitted as ASCII-encoded strings. The maximum expected message size is 1024 bytes.
<br>
**File Transmission:** File data is transmitted in binary format. Before sending the file, its size is transmitted as a 64-bit integer (8 bytes), followed by the file content.

## Usage
Server Setup: Compile and run the server executable on a machine with network access. <br>
Client Connection: Clients should connect to the server using its IP address and port number. <br>
Chat Interaction: Clients can send messages using the chat interface. They can also create/join chat rooms, send files, and leave rooms as desired. <br>
1. Clone the repository:
```sh
  git clone https://github.com/KolomiietsOleksandr/Client-Server-Concepts
  cd Client-Server-Concepts
```
2. Compile the server application:
```sh
  g++ server.cpp -o server
```

3. Run the server.
4. The server will be listening on the specified port (default is 12345).
5. Compile the client application:
```sh
  g++ client.cpp -o client
```
6. Run the client:
7. Enter your name to create a directory.
8. The client will connect to the server. Follow the prompts to interact with the server.

## Commands

| Command | Description | Sending size (in bytes) | Receiving size (in bytes) |
| ------ | ------ | ------ | ------ |
| **CREATE_ROOM <roomname>** | <div> Create new chatroom. </div> | 13  + roomname length | 26 |
| **JOIN_ROOM <roomname>** | <div> Join existing room. </div>  | 11  + roomname length | 102 |
| **LIST_ROOMS** | <div> Get a list of rooms on the server. </div> | 11 | Response length |
| **LEAVE_ROOM** | <div> Leave from room to main menu. </div> | 12 | Response length |
| **/m <message** | <div> Send message to user clients in the room. </div> | 4 + message length | name length + message length |
| **/f <filename** | <div> Send file to user clients in the room. </div> | 4 + filename length + file lenght | Response length |
| **/y** | <div> Accept file transfering </div> | 3 | Response length |
| **/n** | <div> Reject file transfering </div> | 3 | Response length |

## Data Chunking:

To optimize data transfer efficiency, the protocol utilizes data chunking techniques.
Large files are divided into smaller chunks [1024 bytes] before transmission, reducing the risk of network congestion and improving overall performance.
Chunking also allows for better error recovery and resumption of interrupted transfers, as individual chunks can be resent independently.

## Mutex Usage:

The protocol employs mutexes to ensure thread-safe access to shared resources, such as client directories and file manipulation operations.
Mutexes prevent concurrent threads from accessing critical sections of code simultaneously, avoiding race conditions and data corruption.
In multithreaded environments, mutexes are strategically placed to synchronize access to shared data structures and maintain data integrity.
By using mutexes, the protocol guarantees that only one thread can modify shared resources at any given time, enhancing the stability and reliability of the system.

## Diagrams
<br>
![image](https://cdn.discordapp.com/attachments/856234593153056781/1207974633412759572/Untitled-1.png?ex=65e1990b&is=65cf240b&hm=30981ca784a2388d0d67bee8e40fa33f3844c98f51bd111233e9a9c250e6ea16&")


