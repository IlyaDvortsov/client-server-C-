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
		cout << "getaddrinfo failed with error: " << result << endl;
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
		cout << "Socket creation failed " << endl;
		FreeAddrInfo(addrResult);
		WSACleanup();
		return 1;
	}
	
	return 0;
}

int CloseSocket_CleanMem(SOCKET& socket, ADDRINFO* addrResult)
{
	closesocket(socket);
	socket = INVALID_SOCKET;
	FreeAddrInfo(addrResult);
	WSACleanup();

	return 0;
}


int main()
{
	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	SOCKET ConnectSocket = INVALID_SOCKET;

	const char* sendBuffer = "Hello from Client!";

	char recvBuffer[512];

	int result;


	// Инициализация библиотеки для работы с сокетами
	result =  WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cout << "WSAStartup failed, result = " << result << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints)); // Обнуление используемой памяти
	hints.ai_family = AF_INET; // Семейство протокола (IP 4.0 - Ethernet)
	hints.ai_socktype = SOCK_STREAM; // Тип сокета - поток
	hints.ai_protocol = IPPROTO_TCP; // Тип протокола подключения - поток


	result = getaddrinfo("localhost", "666", &hints, &addrResult);
	TestResult(result); //tested
	

	ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	TestConnection(ConnectSocket, addrResult); //tested


	result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		cout << "Unable connect to server " << endl;
		CloseSocket_CleanMem(ConnectSocket, addrResult);
		return 1;
	}


	result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
	if (result == SOCKET_ERROR)
	{
		cout << "send failed, error: " << result << endl;
		CloseSocket_CleanMem(ConnectSocket, addrResult);
		return 1;
	}

	// Число отосланных байтов
	cout << "Bytes sent: " << result << endl;

	result = shutdown(ConnectSocket,SD_SEND);
	if (result == SOCKET_ERROR)
	{
		cout << "Shutdown error " << result << endl;
		CloseSocket_CleanMem(ConnectSocket, addrResult);
		return 1;
	}

	do
	{
		ZeroMemory(recvBuffer, 512);

		result = recv(ConnectSocket, recvBuffer, 512, 0);
		if (result > 0)
		{
			cout << "Received " << result << " bytes" << endl;
			cout << "Received data: " << recvBuffer << endl;
		}
		else if (result == 0)
			cout << "Connection closed " << endl;
		else
			cout << "recv failed with error " << endl;
		} while (result > 0);

		CloseSocket_CleanMem(ConnectSocket, addrResult);
		return 0;
} 


