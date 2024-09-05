#include <iostream>          // Include the C++ standard input/output library.
#include <cstring>           // Include the C string manipulation library.
#include <unistd.h>          // Include the Unix standard library for system calls.
#include <arpa/inet.h>       // Include the library for handling IP addresses and ports.
#include <sys/socket.h>      // Include the socket programming library.
#include <ctime>             // Include the C time library for timestamp generation.
#include <cstdlib>           // Include the C standard library for general functions.
#include <sys/select.h>      // Include the library for socket I/O multiplexing.
#include <cstdio>            // Include the C standard input/output library.

int main() {
    int clientSocket;               // Declare an integer for the client's socket file descriptor.
    struct sockaddr_in serverAddr;  // Declare a struct for the server's address information.

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);  // Create a socket with IPv4, TCP, and protocol 0.
    if (clientSocket == -1) {
        perror("Error in socket");  // Check if the socket creation was successful. If not, print an error message.
        exit(EXIT_FAILURE);         // Exit the program with an error status.
    }

    serverAddr.sin_family = AF_INET;          // Set the address family to IPv4.
    serverAddr.sin_port = htons(12345);       // Set the server's port number to 12345 (convert to network byte order).
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Set the server's IP address to localhost (127.0.0.1).

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error in connect");  // Attempt to connect to the server. If unsuccessful, print an error message.
        exit(EXIT_FAILURE);         // Exit the program with an error status.
    }

    int expectedSeqNum = 0;  // Initialize a variable to keep track of the expected sequence number.

    while (true) {  // Start an infinite loop for communication with the server.
        char message[1024];             // Declare a character array to store received messages.
        memset(message, 0, sizeof(message));  // Initialize the message array to all zeros.
        ssize_t bytesReceived = recv(clientSocket, message, sizeof(message), 0);  // Receive a message from the server.

        if (bytesReceived <= 0) {  // Check if no bytes were received or the connection was closed.
            std::cerr << "Connection closed." << std::endl;  // Print a message indicating the connection is closed.
            break;  // Exit the loop.
        }

        std::cout << "Received Message: " << message << std::endl;  // Print the received message.

        char ack[1024];  // Declare a character array to store acknowledgment messages.
        snprintf(ack, sizeof(ack), "ACK %d", expectedSeqNum);  // Create an acknowledgment message with the expected sequence number.

        struct timeval timeout;  // Declare a struct to specify the timeout for select().
        timeout.tv_sec = 1;     // Set the timeout to 1 second.
        timeout.tv_usec = 0;
        fd_set read_fds;   // Declare a file descriptor set for read operations.
        FD_ZERO(&read_fds);  // Initialize the set to have no file descriptors.
        FD_SET(clientSocket, &read_fds);  // Add the client socket to the set.

        int ready = select(clientSocket + 1, &read_fds, NULL, NULL, &timeout);  // Use select() to check for incoming data or a timeout.
        if (ready == -1) {
            perror("Error in select");  // Check for errors in the select() call and print an error message if needed.
            exit(EXIT_FAILURE);  // Exit the program with an error status.
        } else if (ready == 0) {
            std::cout << "Timeout occurred. Resending acknowledgment." << std::endl;  // Print a message if a timeout occurs.
        } else {
            send(clientSocket, ack, strlen(ack), 0);  // Send the acknowledgment message to the server.
            expectedSeqNum++;  // Increment the expected sequence number.
        }
    }

    close(clientSocket);  // Close the client socket.
    return 0;  // Return 0 to indicate successful program execution.
}
