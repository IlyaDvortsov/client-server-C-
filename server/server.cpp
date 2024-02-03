#define WIN32_LEAN_AND_MEAN // включать при использовании сетевого программирования (протоколы, сокеты и т.д.)


#include<iostream>
#include<Windows.h> // подключение библиотеки, т.к. идет работа с windows
#include<WinSock2.h> // подключение библиотеки для работы с сокетами
#include<WS2tcpip.h> // подключение библиотеки для работы с протоколами


using namespace std;

int TestResult(int result)
{
	// Test getaddrinfo
	if (result != 0)
	{
		cerr << "getaddrinfo failed with error: " << result << endl;
		WSACleanup();
		return 1;
	}

	return 0;
}


int TestConnection(SOCKET& socket, ADDRINFO* addrResult)
{
	// Test Socket 
	if (socket == INVALID_SOCKET)
	{
		cerr << "Socket creation failed " << endl;
		FreeAddrInfo(addrResult);
		WSACleanup();
		return 1;
	}

	return 0;
}

int CloseSocket_CleanMem(SOCKET& socket, ADDRINFO*& addrResult)
{
	closesocket(socket);
	socket = INVALID_SOCKET;
	FreeAddrInfo(addrResult);
	WSACleanup();

	addrResult = nullptr;

	return 0;
}


int main()
{
	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	SOCKET ClientSocket = INVALID_SOCKET; // клиентский сокет
	SOCKET ListenSocket = INVALID_SOCKET;

	const char* sendBuffer = "Hello from Server!";

	char recvBuffer[512];

	int result;


	// Инициализация библиотеки для работы с сокетами
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cerr << "WSAStartup failed, result = " << result << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints)); // Обнуление используемой памяти
	hints.ai_family = AF_INET; // Семейство протокола (IP 4.0 - Ethernet)
	hints.ai_socktype = SOCK_STREAM; // Тип сокета - поток
	hints.ai_protocol = IPPROTO_TCP; // Тип протокола подключения - поток
	hints.ai_flags = AI_PASSIVE; // Сервер - пассивная сторона (ждет соединения), серверный слушающий сокет


	result = getaddrinfo(NULL, "666", &hints, &addrResult); // NULL - т.к. адрес клиента неизвестен
	TestResult(result); //tested


	ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	TestConnection(ListenSocket, addrResult); //tested


	result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen); // привязка сокета
	if (result == SOCKET_ERROR)
	{
		cerr << "Binding socket failed " << endl;
		CloseSocket_CleanMem(ListenSocket, addrResult);
		return 1;
	}

	result = listen(ListenSocket, SOMAXCONN); // Объявление сокета слушающим (слушающий сокет, макс. кол-во соединений); listen - блокирующая операция
	if (result == SOCKET_ERROR)
	{
		cerr << "Listening socket failed " << endl;
		CloseSocket_CleanMem(ListenSocket, addrResult);
		return 1;
	}

	ClientSocket = accept(ListenSocket, NULL, NULL); /// accept - возвращает сокет, cоединенный с клиентом
	if (ClientSocket == INVALID_SOCKET)
	{
		cerr << "Accepting socket failed " << endl;
		CloseSocket_CleanMem(ListenSocket, addrResult);
		return 1;
	}

	closesocket(ListenSocket); // Закрываем слушающий сокет

	do
	{
		ZeroMemory(recvBuffer, 512);

		result = recv(ClientSocket, recvBuffer, 512, 0);
		if (result > 0)
		{
			cout << "Received " << result << " bytes" << endl;
			cout << "Received data: " << recvBuffer << endl;

			result = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0); // Отправка ответа клиенту
			if (result == SOCKET_ERROR)
			{
				cerr << "Failed to send data back " << endl;
				CloseSocket_CleanMem(ClientSocket, addrResult);
			}

		}
		else if (result == 0)
		{
			cout << "Connection closing... " << endl;
		}
		else
		{
			cout << "recv failed with error " << endl;
			CloseSocket_CleanMem(ClientSocket, addrResult);
			return 1;
		}

	} while (result > 0);

	result = shutdown(ClientSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		cout << "shutdown client socket failed " << endl;
		CloseSocket_CleanMem(ClientSocket, addrResult);
	}

	CloseSocket_CleanMem(ClientSocket, addrResult);
	return 0;
}