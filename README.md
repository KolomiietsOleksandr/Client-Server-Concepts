# Client-Server Interaction

This repository demonstrates a simple client-server interaction for managing files on the server. The server allows clients to perform various operations like uploading, downloading, deleting files, getting file information, and listing files. This README provides instructions on using the client and server applications and details possible errors.

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
3. Enter your name to create a directory.
4. The client will connect to the server. Follow the prompts to interact with the server.

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



| Command | Description |
| ------ | ------ |
| **PUT <filename>** | <div> Upload a file to the server.<br>Example: 'PUT myfile.txt' </div> |
| **GET <filename>** | <div>Download a file from the server.<br>Example: 'GET myfile.txt'</div>  |
| **DELETE <filename>** | <div>Delete a file on the server.<br>Example: 'DELETE myfile.txt'</div> |
| **INFO <filename>** | <div>Get information about a file on the server.<br>Example: 'INFO myfile.txt'</div> |
| **LIST** | <div>Get a list of files on the server.</div> |
