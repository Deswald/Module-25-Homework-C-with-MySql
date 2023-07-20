#include "chat.hpp"
 
char buffer[MESSAGE_BUFFER];
char message[MESSAGE_BUFFER];
int message_size;

int chat(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	const char* sendbuf = "this is a test";
	char recvbuf[MESSAGE_BUFFER];
	int iResult;
	int recvbuflen = MESSAGE_BUFFER;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	int options;
	char signUp[MESSAGE_BUFFER] = "signUp";
	char signIn[MESSAGE_BUFFER] = "signIn";
	char usersFull[MESSAGE_BUFFER] = "usersFull";
	char successfulUserAuthorization[MESSAGE_BUFFER] = "successfulUserAuthorization";
	char logOut[MESSAGE_BUFFER] = "logOut";
	char checkingUserMessages[MESSAGE_BUFFER] = "checkingUserMessages";
	char checkingUsers[MESSAGE_BUFFER] = "checkingUsers";

	char myLogin[MESSAGE_BUFFER];
	char myPassword[MESSAGE_BUFFER];

	bool auth = false;
	bool open = true;
	while (open)
	{
		std::cout << "Welcome to the chat, select an operation: " << std::endl;
		std::cout << "1 - to make three users" << std::endl;
		std::cout << "2 - to sign in" << std::endl;
		std::cout << "0 - to quit" << std::endl;


		std::cin >> options;

		switch (options)
		{
		case 1:
			iResult = send(ConnectSocket, signUp, MESSAGE_BUFFER, 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				break;
			}
			std::cout << "Sign Up..." << std::endl;

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			std::cout << "Message from server: " << recvbuf << " Size: " << sizeof(recvbuf) << std::endl;
			recvbuf[iResult-1] = '\0';
			if (strcmp(recvbuf, usersFull) == 0)
			{
				std::cout << "Maximum number of users reached" << std::endl;
				break;
			}
			break;
		case 2:
			iResult = send(ConnectSocket, signIn, MESSAGE_BUFFER, 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				break;
			}
			std::cout << "Sign In..." << std::endl;

			std::cout << "Enter login" << std::endl;
			std::cin >> myLogin;
			iResult = send(ConnectSocket, myLogin, (int)strlen(myLogin), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				break;
			}

			std::cout << "Enter password" << std::endl;
			std::cin >> myPassword;
			iResult = send(ConnectSocket, myPassword, (int)strlen(myPassword), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				break;
			}

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			std::cout << "Message from server: " << recvbuf << " Size: " << sizeof(recvbuf) << std::endl;
			recvbuf[iResult-1] = '\0';
			if (strcmp(recvbuf, successfulUserAuthorization) == 0)
			{
				std::cout << "Successful authorization!" << std::endl;
				auth = true;
				while (auth)
				{
					std::cout << "Hello, select an operation: " << std::endl;
					std::cout << "1 - check messages" << std::endl;
					std::cout << "2 - check users" << std::endl;
					std::cout << "0 - to log out" << std::endl;

					std::cin >> options;

					switch (options)
					{
					 case 1:
						 std::cout << "Checking messages.." << std::endl;
						 
						 iResult = send(ConnectSocket, checkingUserMessages, MESSAGE_BUFFER, 0);
						 if (iResult == SOCKET_ERROR) {
							 printf("send failed with error: %d\n", WSAGetLastError());
							 break;
						 }

						 for (int i = 0; i < 6; i++)
						 {
							 message_size = recv(ConnectSocket, buffer, MESSAGE_BUFFER, 0);
							 buffer[message_size] = '\0';
							 std::cout << "Message: " << buffer << std::endl;
						 }
						 break;
					 case 2:
						 std::cout << "Checking users.." << std::endl;
						 iResult = send(ConnectSocket, checkingUsers, MESSAGE_BUFFER, 0);
						 if (iResult == SOCKET_ERROR) {
							 printf("send failed with error: %d\n", WSAGetLastError());
							 break;
						 }

						 for (int i = 0; i < 3; i++)
						 {
							 message_size = recv(ConnectSocket, buffer, MESSAGE_BUFFER, 0);
							 buffer[message_size] = '\0';
							 std::cout << "User: " << buffer << std::endl;
						 }
						 break;
					case 0:
						std::cout << "Logging out.." << std::endl;
						auth = 0;
						iResult = send(ConnectSocket, logOut, MESSAGE_BUFFER, 0);
						if (iResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							break;
						}
						break;
					default:
						break;
					}
				}
			}
			else
			{
				std::cout << "Incorrect login or password" << std::endl;
				break;
			}
			break;
		case 0:
			open = false;

			// shutdown the connection since no more data will be sent
			iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}

			std::cout << "Client work is done.!" << std::endl;

			// cleanup
			closesocket(ConnectSocket);
			WSACleanup();
			exit(0);
			break;
		default:
			break;
		}
	}
}