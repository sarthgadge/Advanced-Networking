
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

const int MAX_CLIENTS = 10;
map<string, int> clientSockets;// associates usernames (strings) with client socket descriptors (integers)
pthread_mutex_t clientMutex = PTHREAD_MUTEX_INITIALIZER;// safely accessing the clientSockets map in a multithreaded environment

// Function to handle a single client connection
// function that handles each connected client
void *HandleClient(void *arg)
{
    int clientSocket = *(int *)arg;
    char buffer[1024];
    char username[50];
    int bytesRead;
    bzero(buffer, sizeof(buffer));
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0)
    {
        // Client disconnected or error
        close(clientSocket);
        pthread_exit(NULL);
    }

    strncpy(username, buffer, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';
    printf("%s joined\n", username);

    // Lock the mutex to safely access the clientSockets map
    pthread_mutex_lock(&clientMutex);//locks the clientMutex to safely access and modify the clientSockets map
    clientSockets[username] = clientSocket;// adds the client's username and socket descriptor to the clientSockets map
    pthread_mutex_unlock(&clientMutex);// unlocks the mutex to release the lock
    //this loop continuously runs between client and server
    while (true)
    {
        bzero(buffer, sizeof(buffer));//resets the buffer and receives messages from the client
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            // Client disconnected or error
            printf("%s disconnected\n", username);
            pthread_mutex_lock(&clientMutex);//locks the clientMutex 
            auto it = clientSockets.find(username);
            if (it != clientSockets.end())
            {
                clientSockets.erase(it);//removes the disconnected client from the clientSockets map
            }
            pthread_mutex_unlock(&clientMutex);// and unlocks the mutex.
            close(clientSocket);
            break;
        }
        char user[50];//store the sender's username and the message content.
        char message[1024];
        bzero(user, sizeof(user));// resets both buffers using bzero
        bzero(message, sizeof(message));
        sscanf(buffer, "%49[^:]:%1023[^\n]", user, message);

        cout << user << endl;

        // Forward the received message to all other connected clients
        char username_buffer[1074]; // Adjust the size accordingly
        strcpy(username_buffer, username);//prepares a message buffer, username_buffer, to forward the received message to other clients.
        strcat(username_buffer, ": ");

        if (strcmp(buffer, "/exit") == 0)
        {
            printf("%s disconnected\n", username);
            strcat(username_buffer, "disconnected");

            pthread_mutex_lock(&clientMutex);
            // iterates through all connected clients
            for (const auto &entry : clientSockets)
            {
                if (entry.first != username)//if the client is not the sender 
                {
                    int bytesSent = send(entry.second, username_buffer, strlen(username_buffer), 0);//target user matches the recipient's
                    if (bytesSent < 0)                                                              // username or if the target user is "_"
                                                                                                    // it sends the message to the recipient.
                    {
                        cerr << "Error sending message to " << entry.first << endl;
                    }
                }
            }
            clientSockets.erase(username);
            pthread_mutex_unlock(&clientMutex);
            close(clientSocket);
            break;
        }

        strcat(username_buffer, message);

        pthread_mutex_lock(&clientMutex);
        bool user_found = 0;
        for (const auto &entry : clientSockets)
        {
            if (entry.first != username)
            {
                if (user == entry.first)
                {
                    user_found = 1;
                    int bytesSent = send(entry.second, username_buffer, strlen(username_buffer), 0);
                    if (bytesSent < 0)
                    {
                        cerr << "Error sending message to " << entry.first << endl;
                    }
                }
                else
                {
                    if (strcmp(user, "_") == 0)
                    {
                        user_found = 1;
                        int bytesSent = send(entry.second, username_buffer, strlen(username_buffer), 0);
                        if (bytesSent < 0)
                        {
                            cerr << "Error sending message to " << entry.first << endl;
                        }
                    }
                }
            }
        }
        if (user_found == 0)
        {
            int bytesSent = send(clientSocket, "user not found\n", strlen("user not found\n"), 0);
            if (bytesSent < 0)
            {
                cerr << "Error sending message to " << username << endl;
            }
        }
        pthread_mutex_unlock(&clientMutex);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << "<Server_Port_Number>" << endl;
        return 1;
    }
    const int SERVER_PORT = atoi(argv[1]);
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(clientAddr);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        cerr << "Error creating server socket." << endl;
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)//binds the server socket to the specified address and port
    {
        cerr << "Error binding server socket." << endl;
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1)
    {
        cerr << "Error listening on server socket." << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Server listening on port " << SERVER_PORT << endl;

    while (true)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrSize);
        if (clientSocket == -1)
        {
            cerr << "Error accepting client connection." << endl;
            continue;
        }

        pthread_t clientThread;
        if (pthread_create(&clientThread, nullptr, HandleClient, &clientSocket) != 0)
        {
            cerr << "Error creating client thread." << endl;
            close(clientSocket);
        }
    }

    close(serverSocket);

    return 0;
}
