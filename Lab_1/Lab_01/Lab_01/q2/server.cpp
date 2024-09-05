
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0
#define PORT 5000

int main(int argc, char *argv[])
{
	int opt = TRUE;
	int master_soc, addrlen, new_soc, client_soc[30] = {0},
																		maximum_clients = 30, activity, i, valread, sd;
	int maximum_sd;
	struct sockaddr_in address;

	char buffer[1025];

	fd_set readfds;

	char *message = "CHAT v1.0 \r\n";

	if ((master_soc = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(master_soc, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
								 sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(master_soc, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("binding failed");
		exit(EXIT_FAILURE);
	}
	printf("Server(listener) on port %d \n", PORT);

	if (listen(master_soc, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	bool first = false;

	while (TRUE)
	{
		FD_ZERO(&readfds);

		FD_SET(master_soc, &readfds);
		FD_SET(STDIN_FILENO, &readfds);
		maximum_sd = master_soc;

		for (i = 0; i < maximum_clients; i++)
		{
			sd = client_soc[i];

			if (sd > 0)
				FD_SET(sd, &readfds);

			if (sd > maximum_sd)
				maximum_sd = sd;
		}

		activity = select(maximum_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			if (first)
			{
				printf("No client connected!\n");
				continue;
			}
			else
			{
				valread = read(STDIN_FILENO, buffer, 1024);
				buffer[valread] = '\0';
				sd = client_soc[(int)(buffer[0] - '0')];
				if (sd == 0)
				{
					printf("No client with the given id.\n");
				}
				else
				{
					send(sd, buffer, strlen(buffer), 0);
				}
				continue;
			}
		}

		if (FD_ISSET(master_soc, &readfds))
		{
			if ((new_soc = accept(master_soc,
														(struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("New connection , socket fd is %d , ip_address is : %s , port : %d\n", new_soc, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			if (send(new_soc, message, strlen(message), 0) != strlen(message))
			{
				perror("send");
			}

			puts("Welcome message sent successfully");

			for (i = 0; i < maximum_clients; i++)
			{
				if (client_soc[i] == 0)
				{
					client_soc[i] = new_soc;
					printf("Added to list of sockets as %d\n", i);

					break;
				}
			}
		}

		for (i = 0; i < maximum_clients; i++)
		{
			sd = client_soc[i];

			if (FD_ISSET(sd, &readfds))
			{
				if ((valread = read(sd, buffer, 1024)) == 0)
				{
					getpeername(sd, (struct sockaddr *)&address,
											(socklen_t *)&addrlen);
					printf("Host disconnected , ip_address was %s , port %d \n",
								 inet_ntoa(address.sin_addr), ntohs(address.sin_port));

					close(sd);
					client_soc[i] = 0;
				}

				else
				{
					buffer[valread] = '\0';
					printf("%s", buffer);
				}
			}
		}
	}

	return 0;
}
