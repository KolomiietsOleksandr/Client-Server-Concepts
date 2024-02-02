# Client-Server Interaction

This repository demonstrates a simple client-server interaction for managing files on the server. The server allows clients to perform various operations like uploading, downloading, deleting files, getting file information, and listing files. This README provides instructions on using the client and server applications and details possible errors.
The communication protocol is implemented as a simple request-response protocol over TCP/IP sockets.

## Server
### Instructions:

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

### Possible Errors:
**Error creating socket:**
- Check if you have necessary permissions to create sockets.
- Ensure that no other process is using the specified port.

**Bind failed:**
- Make sure the specified port is not already in use.

**Listen failed:**
- Ensure there are no issues with your network configuration.

**Error opening directory:**
- Check if the server has the necessary permissions to access the specified file directory.

## Client
### Instructions:
1. Compile the client application:
```sh
  g++ client.cpp -o client
```
2. Run the client:
3. The client will connect to the server. Follow the prompts to interact with the server.

### Possible Errors:
**Error creating socket:**
- Check if you have necessary permissions to create sockets.
- Ensure that the server IP address and port are correctly set.

**Connect failed:**
- Verify that the server is running and reachable.

**Error opening file for saving:**
- Check if the client has the necessary permissions to write files in the specified storage directory.

**Error opening file:**
- Ensure the specified file exists and the client has read permissions.

## Commands

| Command | Description |
| ------ | ------ |
| **PUT <filename>** | <div> Upload a file to the server.<br>Example: 'PUT myfile.txt' </div> |
| **GET <filename>** | <div>Download a file from the server.<br>Example: 'GET myfile.txt'</div>  |
| **DELETE <filename>** | <div>Delete a file on the server.<br>Example: 'DELETE myfile.txt'</div> |
| **INFO <filename>** | <div>Get information about a file on the server.<br>Example: 'INFO myfile.txt'</div> |
| **LIST** | <div>Get a list of files on the server.</div> |

<br>

### Processes
**Connection Management:**
The client initiates a connection to the server upon startup.
Both client and server maintain the connection until explicitly terminated.

**Concurrency Handling:**
The server handles multiple client connections concurrently using multithreading.
Each client request is processed independently to ensure responsiveness and scalability.

**Security Considerations:**
Currently, the protocol does not include authentication or encryption mechanisms.

### Data Chunking:

To optimize data transfer efficiency, the protocol utilizes data chunking techniques.
Large files are divided into smaller chunks [1024 bytes] before transmission, reducing the risk of network congestion and improving overall performance.
Chunking also allows for better error recovery and resumption of interrupted transfers, as individual chunks can be resent independently.

### Mutex Usage:

The protocol employs mutexes to ensure thread-safe access to shared resources, such as client directories and file manipulation operations.
Mutexes prevent concurrent threads from accessing critical sections of code simultaneously, avoiding race conditions and data corruption.
In multithreaded environments, mutexes are strategically placed to synchronize access to shared data structures and maintain data integrity.
By using mutexes, the protocol guarantees that only one thread can modify shared resources at any given time, enhancing the stability and reliability of the system.

<br>

## Diagrams
![image](https://github.com/KolomiietsOleksandr/Client-Server-Concepts/assets/115143584/df772af5-6e5a-4fe8-99f2-d3c74ed04538)
