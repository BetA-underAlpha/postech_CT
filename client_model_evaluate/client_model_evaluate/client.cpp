#include"client.h"
#include"ai.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <time.h>

void initClient(socketSet *socketInfo)
{
	int port;
	char IP[IP_SIZE];

	getInfo(&port, IP);

	if (WSAStartup(MAKEWORD(2, 2), &(socketInfo->wsaData)) != 0)
		ErrorHandling("WSAStartup() error!");

	(socketInfo->clientSock) = socket(PF_INET, SOCK_STREAM, 0);

	memset(&(socketInfo->serverAddr), 0, sizeof((socketInfo->serverAddr)));
	(socketInfo->serverAddr).sin_family = AF_INET;
	(socketInfo->serverAddr).sin_addr.s_addr = inet_addr(IP);
	(socketInfo->serverAddr).sin_port = htons(port);

	if (connect((socketInfo->clientSock), (SOCKADDR*)&(socketInfo->serverAddr), sizeof((socketInfo->serverAddr))) == SOCKET_ERROR)
		ErrorHandling("connect() error");
}

void runClient(socketSet *socketInfo)
{
	AI(socketInfo->clientSock);
}

void endClient(socketSet *socketInfo)
{
	closesocket((socketInfo->clientSock));
	WSACleanup();
}

void getInfo(int *port, char *IP)
{
	printf("port> ");
	scanf("%d", port);
	printf("IP> ");
	scanf("%s", IP);
}

void ErrorHandling(const char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}