#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <string>
#include <iostream>

static const unsigned char base64encoding_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(unsigned char *src, size_t length)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olength;

    olength = 4 * ((length + 2) / 3);

    char *outStr = new char[0];

    if (olength < length)
        return NULL;

    outStr = new char[olength];
    out = (unsigned char *)&outStr[0];

    end = src + length;
    in = src;
    pos = out;
    while (end - in >= 3)
    {
        *pos++ = base64encoding_table[in[0] >> 2];
        *pos++ = base64encoding_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64encoding_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64encoding_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in)
    {
        *pos++ = base64encoding_table[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = base64encoding_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = base64encoding_table[((in[0] & 0x03) << 4) |
                                          (in[1] >> 4)];
            *pos++ = base64encoding_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}

#define MSG_LENGTH 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IP_Address> <Port_Number>\n", argv[0]);
        return 1;
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);

    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(soc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("connect");
        return 1;
    }

    printf("Connected to server\n");
    char buffer[MSG_LENGTH];
    int bytesRead = recv(soc, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl;
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        printf("Server Acknowledgment: %s\n", buffer);
    }

    while (1)
    {
        char message[MSG_LENGTH];
        printf("Enter a message, format:<type of message(1-normal, 2-acknowlegement, 3-exit)> <message>: ");
        fgets(message, MSG_LENGTH, stdin);

        char *formattedMessage = base64_encode((unsigned char *)message, strlen(message));

        send(soc, formattedMessage, strlen(formattedMessage), 0);
        if (message[0] == '3')
            break;

        char buffer[MSG_LENGTH];
        int siz = recv(soc, buffer, MSG_LENGTH, 0);
        if (siz > 0)
        {
            std::cout << "Acknowledgement recieved(sent by server): " << buffer << std::endl;
        }
    }

    close(soc);
    printf("Connection closed\n");

    return 0;
}
