#include "chat.hpp"
char buffer[MESSAGE_BUFFER]; 
int message_size;
const char *endSession_string = "endSession";
const char *signUp_string = "signUp";
const char *signIn_string = "signIn";
const char *checkingUserMessages_string = "checkingUserMessages";
const char* checkingUsers_string = "checkingUsers";
const char *logOut_string = "logOut";
char bufferName[MESSAGE_BUFFER];
char bufferLogin[MESSAGE_BUFFER]; 
char bufferPassword[MESSAGE_BUFFER]; 

int chat()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[MESSAGE_BUFFER];
	int recvbuflen = MESSAGE_BUFFER;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for the server to listen for client connections.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "The server has created a socket and accept a client socket!" << std::endl;

	// No longer need server socket
	closesocket(ListenSocket);

	const int usersCount = 3;
	bool auth = false;
	static int index = 0;
	char usersFull[MESSAGE_BUFFER] = "usersFull";
	char usersNotFull[MESSAGE_BUFFER] = "usersNotFull";
	char successfulUserAuthorization[MESSAGE_BUFFER] = "successfulUserAuthorization";
	char failedUserAuthorization[MESSAGE_BUFFER] = "failedUserAuthorization";

	// Connect to database
	MYSQL mysql;
	MYSQL_RES* res;
	MYSQL_ROW row;
	std::string currentUser;

	mysql_init(&mysql);
	if (&mysql == nullptr) {
		std::cout << "Error: can't create MySQL-descriptor" << std::endl;
	}

	if (!mysql_real_connect(&mysql, "localhost", "root", "des_2023_wald", "chat_mysql", NULL, NULL, 0)) {
		std::cout << "Error: can't connect to database " << mysql_error(&mysql) << std::endl;
	}
	else {
		std::cout << "Success!" << std::endl;
	}

	mysql_set_character_set(&mysql, "utf8");
	std::cout << "connection characterset: " << mysql_character_set_name(&mysql) << std::endl;

	do {

		std::cout << "The server is waiting for a message.." << std::endl;
		iResult = recv(ClientSocket, recvbuf, MESSAGE_BUFFER, 0);

		if (iResult > 0) {
			recvbuf[iResult - 1] = '\0';
			if (strcmp(recvbuf, signUp_string) == 0)
			{
				if (index < usersCount)
				{
					iSendResult = send(ClientSocket, usersNotFull, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}

					mysql_query(&mysql, "INSERT INTO users (id, name, login, password) VALUES (1, 'Vasya', 'vas', 'vasYA')");
					mysql_query(&mysql, "INSERT INTO users (id, name, login, password) VALUES (2, 'Kolya', 'kol', 'kolYA')");
					mysql_query(&mysql, "INSERT INTO users (id, name, login, password) VALUES (3, 'Petya', 'pe', 'petYA')");

					mysql_query(&mysql, "INSERT INTO messages (m_id, sender_id, receiver_id, text_m) VALUES (default, '1', '2', 'lol')");
					mysql_query(&mysql, "INSERT INTO messages (m_id, sender_id, receiver_id, text_m) VALUES (default, '1', '3', 'omg')");
					
					mysql_query(&mysql, "INSERT INTO messages (m_id, sender_id, receiver_id, text_m) VALUES (default, '2', '1', 'lmao')");
					mysql_query(&mysql, "INSERT INTO messages (m_id, sender_id, receiver_id, text_m) VALUES (default, '2', '3', 'Hello')");

					mysql_query(&mysql, "INSERT INTO messages (m_id, sender_id, receiver_id, text_m) VALUES (default, '3', '1', 'Greetings')");
					mysql_query(&mysql, "INSERT INTO messages (m_id, sender_id, receiver_id, text_m) VALUES (default, '3', '2', 'Hi')");

					index += 3;
				}
				else
				{
					std::cout << "Maximum number of users reached: " << usersCount << std::endl;
					iSendResult = send(ClientSocket, usersFull, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}

				}
			}
			// Authorization
			if (strcmp(recvbuf, signIn_string) == 0)
			{
				std::cout << "The server is authenticating.." << std::endl;

				message_size = recv(ClientSocket, bufferLogin, MESSAGE_BUFFER, 0);
				bufferLogin[message_size] = '\0';
				std::cout << "Message Received from Client (login): " << bufferLogin << std::endl;

				message_size = recv(ClientSocket, bufferPassword, MESSAGE_BUFFER, 0);
				bufferPassword[message_size] = '\0';
				std::cout << "Message Received from Client (password): " << bufferPassword << std::endl;

				mysql_query(&mysql, "SELECT login FROM users");
				if (res = mysql_store_result(&mysql)) {
					std::string checkLogin = bufferLogin;
					std::string checkPassword = bufferPassword;
					while (row = mysql_fetch_row(res)) {
						for (int i = 0; i < mysql_num_fields(res); i++) {
							if (row[i] == checkLogin)
							{
								std::cout << "CORRECT LOGIN" << std::endl;

								mysql_query(&mysql, "SELECT password FROM users");
								if (res = mysql_store_result(&mysql)) {
									while (row = mysql_fetch_row(res)) {
										for (int y = 0; y < mysql_num_fields(res); y++) {
											if (row[y] == checkPassword)
											{
												std::cout << "CORRECT PASSWORD" << std::endl;
												currentUser = bufferLogin;
												auth = true;
												break;
											}
										}
									}
								}
								else
									std::cout << "Ошибка MySql номер " << mysql_error(&mysql);
							}
						}
					}
				}
				else
					std::cout << "Ошибка MySql номер " << mysql_error(&mysql) << std::endl;
				if (auth)
				{
					std::cout << "Successful authorization!" << std::endl;

					iSendResult = send(ClientSocket, successfulUserAuthorization, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}

					// Authorized user menu
					while (auth)
					{
						std::cout << "Waiting for an authorized user to select an action.." << std::endl;

						iResult = recv(ClientSocket, recvbuf, MESSAGE_BUFFER, 0);
						recvbuf[iResult - 1] = '\0';

						// Log out
						if (strcmp(recvbuf, logOut_string) == 0)
						{
							std::cout << "Logging out.." << std::endl;
							auth = 0;
						}
						// Check messages
						if (strcmp(recvbuf, checkingUserMessages_string) == 0)
						{
							mysql_query(&mysql, "SELECT text_m FROM messages;");
							if (res = mysql_store_result(&mysql)) {
								char* charMessage;

								while (row = mysql_fetch_row(res)) {
									for (int i = 0; i < mysql_num_fields(res); i++) {
										std::cout << row[i] << "  ";
										charMessage = row[i];
										iSendResult = send(ClientSocket, charMessage, sizeof(charMessage), 0);
										if (iSendResult == SOCKET_ERROR) {
											printf("send failed with error: %d\n", WSAGetLastError());
										}

									}
									std::cout << std::endl;
								}
							}
							else
								std::cout << "Ошибка MySql номер " << mysql_error(&mysql);
						}
						if (strcmp(recvbuf, checkingUsers_string) == 0)
						{
							mysql_query(&mysql, "SELECT name FROM users;");
							if (res = mysql_store_result(&mysql)) {
								char* charName;

								while (row = mysql_fetch_row(res)) {
									for (int i = 0; i < mysql_num_fields(res); i++) {
										std::cout << row[i] << "  ";
										charName = row[i];
										iSendResult = send(ClientSocket, charName, sizeof(charName), 0);
										if (iSendResult == SOCKET_ERROR) {
											printf("send failed with error: %d\n", WSAGetLastError());
										}

									}
									std::cout << std::endl;
								}
							}
							else
								std::cout << "Ошибка MySql номер " << mysql_error(&mysql);

						}
					}
				}
				else
				{
					std::cout << "Incorrect login or password" << std::endl;

					iSendResult = send(ClientSocket, failedUserAuthorization, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}
				}
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	std::cout << "Server is Quitting" << std::endl;

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	// Closing the database connection
	mysql_close(&mysql);
	system("Pause");

	return 0;
}