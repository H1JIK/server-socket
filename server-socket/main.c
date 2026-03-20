#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define PORT "2200"
#define MAX_MSG 256
#define MAX_PATH 64

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

SOCKET client_s = INVALID_SOCKET;
SOCKET listen_s = INVALID_SOCKET;
WSADATA wsadata;
ADDRINFO hints;
ADDRINFO* result;

void errors_f(int n) {
	if (n >= 3) closesocket(listen_s);
	if (n >= 2 && n < 5) freeaddrinfo(result);
	if (n) WSACleanup();
	switch (n) {
	case 0:
		printf("~err: recv failed\n");
		break;
	case 1:
		printf("~err: WSAStartup failed\n");
		break;
	case 2:
		printf("~err: getaddrinfo failed\n");
		break;
	case 3:
		printf("~err: invalid socket\n");
		break;
	case 4:
		printf("~err: binding socket failed\n");
		break;
	case 5:
		printf("~err: listening socket failed\n");
		break;
	case 6:
		printf("~err: accepting client socket failed\n");
		break;
	case 7:
		printf("goodbye!\n");
		break;
	}

	if (n) exit(0);
}

void main() {
	char sendbuf[MAX_PATH] = {0};
	char recvbuf[MAX_MSG] = {0};

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;	//серверный флаг

	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) errors_f(1);	//передача инфы по системе библиотеке/компилятору

	if (getaddrinfo(NULL, PORT, &hints, &result)) errors_f(2);	//раб-ет как переводчик для сокетов (преобразование различных данных)

	listen_s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_s == INVALID_SOCKET) errors_f(3);

	if (bind(listen_s, result->ai_addr, (int)result->ai_addrlen)) errors_f(4);	//закрепляет адрес за собой

	freeaddrinfo(result);

	if (listen(listen_s, 1) == SOCKET_ERROR) errors_f(5);	//переводит сокет в режим ожидания

	printf("Waiting for connection...\n");

	client_s = accept(listen_s, NULL, NULL);	//блок. функция (синхронная), цикл, пока не подкл
	if (client_s == INVALID_SOCKET) errors_f(6);
	printf("CONNECTING COMPLETE!\n");


	closesocket(listen_s);
	
	int r_bytes;


	do {
		memset(recvbuf, 0, MAX_MSG);

		r_bytes = recv(client_s, recvbuf, (int)MAX_MSG, 0);

		if (r_bytes > 0) {
			if (recvbuf[0] != '\0') printf("MSG: %s\n", recvbuf);

			printf("0 - exit\n");
			printf("Enter the full path of the deleted file: ");
			scanf("%s", sendbuf);
			if (sendbuf[0] == '0')	errors_f(7);
			else{
				if (!send(client_s, sendbuf, (int)strlen(sendbuf), 0)) {
					printf("~err: send failed\n");
					continue;
				}
			}
		}
		else if (r_bytes == 0) printf("Connecton closed!\n");
		else errors_f(0);
	} while (r_bytes > 0);

	errors_f(7);	//очистка
}