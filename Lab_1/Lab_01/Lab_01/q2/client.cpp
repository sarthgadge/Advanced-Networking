#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int soc_fd, portnumber, n, valread, activity;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set readfds;

    char buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "Error: Command usage is \'%s <hostname> <port>\n\'", argv[0]);
        exit(0);
    }
    portnumber = atoi(argv[2]);
    soc_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (soc_fd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portnumber);
    if (connect(soc_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1)
    {

        FD_ZERO(&readfds);
        FD_SET(soc_fd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        activity = select(soc_fd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            valread = read(STDIN_FILENO, buffer, 1024);
            buffer[valread] = '\0';
            send(soc_fd, buffer, strlen(buffer), 0);
            printf("Sent the message\n");
        }
        else
        {
            valread = read(soc_fd, buffer, 1024);
            buffer[valread] = '\0';
            if (strlen(buffer) == 0)
                break;
            printf("%s\n", buffer);
        }
    }

    close(soc_fd);
    return 0;
}