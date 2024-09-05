
#include <iostream>           // Include the iostream library for standard input and output operations.
#include <cstring>            // Include the cstring library for string manipulation functions.
#include <unistd.h>           // Include the unistd.h library for low-level I/O operations, like the close function.
#include <arpa/inet.h>         // Include the arpa/inet.h library for various networking functions and structures.
#include <sys/socket.h>       // Include the sys/socket.h library for socket-related functions and structures.
#include <ctime>              // Include the ctime library for time-related functions.
#include <cstdlib>            // Include the cstdlib library for general-purpose functions.
#include <sys/select.h>       // Include the sys/select.h library for the select function, used for I/O multiplexing.

int main() {
    int serverSocket, clientSocket;           // Declare integer variables for server and client sockets.
    struct sockaddr_in serverAddr, clientAddr; // Declare sockaddr_in structures for server and client addresses.
    socklen_t addrSize;                        // Declare a variable to store the size of addresses.

    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket with AF_INET (IPv4) and SOCK_STREAM (TCP).
    if (serverSocket == -1) {
        perror("Error in socket");             // Check if the socket creation failed and print an error message if it did.
        exit(EXIT_FAILURE);                     // Exit the program with an error status.
    }

    serverAddr.sin_family = AF_INET;            // Set the server address family to IPv4.
    serverAddr.sin_port = htons(12345);         // Set the server port number and convert it to network byte order.
    serverAddr.sin_addr.s_addr = INADDR_ANY;    // Bind the socket to all available network interfaces.

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error in bind");                // Check if binding the socket to the address failed and print an error message if it did.
        exit(EXIT_FAILURE);                     // Exit the program with an error status.
    }

    if (listen(serverSocket, 5) == -1) {
        perror("Error in listen");              // Check if the listen operation failed and print an error message if it did.
        exit(EXIT_FAILURE);                     // Exit the program with an error status.
    }

    std::cout << "Server is waiting for connections..." << std::endl; // Print a message to indicate that the server is waiting for connections.

    addrSize = sizeof(clientAddr);               // Set the address size variable to the size of the client address.
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize); // Accept an incoming connection request from a client.
    if (clientSocket == -1) {
        perror("Error in accept");               // Check if accepting the client connection failed and print an error message if it did.
        exit(EXIT_FAILURE);                     // Exit the program with an error status.
    }

    std::cout << "Client connected." << std::endl; // Print a message to indicate that a client has successfully connected.

    int seqNum = 0;  // Initialize a sequence number to track the messages being sent.

    while (true) {
        char message[1024];  // Declare a character array to store the message to be sent.
        snprintf(message, sizeof(message), "Message with SeqNum %d.", seqNum); // Create a message with the sequence number.

        send(clientSocket, message, strlen(message), 0); // Send the message to the client over the socket.

        struct timeval timeout;        // Declare a struct to represent the timeout.
        timeout.tv_sec = 1;            // Set the timeout to 1 second.
        timeout.tv_usec = 0;
        fd_set read_fds;               // Declare a set of file descriptors for monitoring.
        FD_ZERO(&read_fds);            // Initialize the set to be empty.
        FD_SET(clientSocket, &read_fds); // Add the client socket to the set for monitoring.

        int ready = select(clientSocket + 1, &read_fds, NULL, NULL, &timeout); // Use the select function to wait for client response.
        if (ready == -1) {
            perror("Error in select");     // Check if an error occurred in the select function and print an error message if it did.
            exit(EXIT_FAILURE);             // Exit the program with an error status.
        } else if (ready == 0) {
            std::cout << "Timeout occurred. Resending message." << std::endl; // If there was a timeout, print a message indicating that the message will be resent.
        } else {
            char ack[1024];               // Declare a character array to store the acknowledgment.
            memset(ack, 0, sizeof(ack));  // Initialize the acknowledgment array with zeros.
            ssize_t bytesReceived = recv(clientSocket, ack, sizeof(ack), 0); // Receive an acknowledgment from the client.

            if (bytesReceived <= 0) {
                std::cerr << "Connection closed." << std::endl; // If no bytes were received, indicate that the connection is closed and exit the loop.
                break;
            }

            std::cout << "Received Acknowledgment: " << ack << std::endl; // Print the received acknowledgment.

            int ackSeqNum;
            if (sscanf(ack, "ACK %d", &ackSeqNum) == 1 && ackSeqNum == seqNum) {
                std::cout << "ACK received. Message delivered successfully." << std::endl; // Check the acknowledgment's format and sequence number and print a success message.
            } else {
                std::cout << "NACK received. Resending message." << std::endl; // If the acknowledgment indicates failure, print a message indicating that the message will be resent.
            }

            seqNum++; // Increment the sequence number for the next message.
        }
    }

    close(serverSocket); // Close the server socket.
    return 0;
}
