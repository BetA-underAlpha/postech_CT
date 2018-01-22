#pragma once
#ifdef _DEBUG
#pragma comment(lib, "opencv_world340d.lib")
#else
#pragma comment(lib, "opencv_world340.lib")
#endif
#pragma comment(lib,"ws2_32.lib")

#include <Windows.h>

#define IP_SIZE (50)

typedef struct socketSet {
	WSADATA wsaData;
	SOCKET clientSock;
	SOCKADDR_IN serverAddr;
} socketSet;

void ErrorHandling(const char* msg);

void initClient(socketSet *socketInfo);
void runClient(socketSet *socketInfo);
void endClient(socketSet *socketInfo);
void getInfo(int *port, char *IP);