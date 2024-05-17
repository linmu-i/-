#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>

#pragma comment(lib,"ws2_32.lib")

class server
{
private:

	std::vector<char> byteTmpRecv;

	SOCKET sockServer;
	SOCKET sockTemp[256];

	int port;
	int numOfConnect = 0;
	int len = sizeof(SOCKADDR_IN);
	int len6 = sizeof(SOCKADDR_IN6);

	SOCKADDR_IN ADDRV4;
	SOCKADDR_IN6 ADDRV6;

	SOCKADDR_IN clientAddr[256];

	void svrConnected(void(*getData_b)(), void(*connect_b)(), void(*disconnect_b)(), std::vector<char>* data);
	void svrEvent(void(* getData_c)(), void(* disconnect_c)(),int num, std::vector<char>* data);

public:

	bool creatV4(int port, void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)(), std::vector<char>* data);
	bool creatV6(int port, void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)(), std::vector<char>* data);

	bool sendCharArrToClient(int num, char* byte, int lenOfData);
	bool sendStringToClient(int num, std::vector<char> byte);

};

void server::svrEvent(void(*getData_c)(), void(*disconnect_c)(), int num, std::vector<char>* data)
{
	char tmp[1024];
	int itmp;
	while (1)
	{
		u_long mode = 1;
		ioctlsocket(sockTemp[num], FIONBIO, &mode);
		itmp = recv(sockTemp[num], tmp, 1024, 0);
	
		if (itmp > 0)
		{
			byteTmpRecv.insert(byteTmpRecv.end(), tmp, tmp + itmp);
		}
		else if(itmp < 0 and byteTmpRecv.size() != 0)
		{
			*data = byteTmpRecv;
			getData_c();
			byteTmpRecv.clear();
		}
		else if (itmp < 0 and byteTmpRecv.size() == 0)
		{
			//滚回去循环
		}
		else
		{
			disconnect_c();
			break;
		}
	}
}

void server::svrConnected(void(*getData_b)(), void(*connect_b)(), void(*disconnect_b)(), std::vector<char>* data)
{
	while (1)
	{
		if ((sockTemp[numOfConnect] = accept(sockServer, (SOCKADDR*)(&(clientAddr[numOfConnect])), &len)) != SOCKET_ERROR)
		{
			std::thread thr_attachment_b(&server::svrEvent, this, getData_b, disconnect_b, numOfConnect,data);
			thr_attachment_b.detach();
			connect_b();
		}
	}
}

bool server::creatV4(int port,  void(* getData_a)(), void(* connect_a)(), void(* disconnect_a)(),std::vector<char>* data)
{
	bool test = 1;
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2),&wd ) != 0)
	{
		test = 0;
		return test;
	}

	ADDRV4.sin_family = AF_INET;
	ADDRV4.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ADDRV4.sin_port = htons(port);

	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (bind(sockServer, (sockaddr*)&ADDRV4, len))
	{
		test = 0;
		return test;
	}

	if (listen(sockServer, 5) == SOCKET_ERROR)
	{
		test = 0;
		return test;
	}
	
	std::thread thr_attachment_a(&server::svrConnected, this, getData_a, connect_a, disconnect_a,data);
	thr_attachment_a.detach();

	return test;
}

bool server::creatV6(int port,  void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)(),std::vector<char>* data)
{
	bool test = 1;
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		test = 0;
		return test;
	}

	memset(&ADDRV6, 0, sizeof(SOCKADDR_IN6));
	ADDRV6.sin6_family = AF_INET6;
	ADDRV6.sin6_port = htons(port);
	ADDRV6.sin6_addr = in6addr_any;

	sockServer = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	if (bind(sockServer, (sockaddr*)&ADDRV6, len6))
	{
		test = 0;
		return test;
	}

	if (listen(sockServer, 5) == SOCKET_ERROR)
	{
		test = 0;
		return test;
	}

	std::thread thr_attachment_a(&server::svrConnected, this, getData_a, connect_a, disconnect_a,data);
	thr_attachment_a.detach();

	return test;
}

bool server::sendCharArrToClient(int num, char* data, int lenOfData)
{
	if ((send(sockTemp[num], data, lenOfData, 0)) == SOCKET_ERROR)
		return false;
	else
		return true;
}

bool server::sendStringToClient(int num, std::vector<char> data)
{
	if ((send(sockTemp[num], data.data(), data.size(), 0)) == SOCKET_ERROR)
		return false;
	else
		return true;
}

class client
{
private:

	std::vector<char> byteTmpRecv;

	SOCKET sockClient;

	int len = sizeof(SOCKADDR_IN);
	int len6 = sizeof(SOCKADDR_IN6);

	SOCKADDR_IN ADDRV4;
	SOCKADDR_IN6 ADDRV6;

	SOCKADDR_IN clientAddr[256];

	void svrEvent(void(*getData_c)(), void(*disconnect_c)());

public:

	bool creatV4(char IP[], int port, void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)());
	bool creatV6(char IP[], int port, void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)());

	bool sendCharArrToServer(int num, char* byte, int lenOfData);
	bool sendStringToServer(int num, std::vector<char> byte);

};

void client::svrEvent(void(*getData_c)(), void(*disconnect_c)())
{
	char tmp[1024];
	while (1)
	{
		u_long mode = 1;
		ioctlsocket(sockClient, FIONBIO, &mode);
		int itmp = recv(sockClient, tmp, 1024, 0);

		if (itmp > 0)
		{
			byteTmpRecv.insert(byteTmpRecv.end(), tmp, tmp + 1024);
		}
		else if (itmp < 0 and byteTmpRecv.size() != 0)
		{
			getData_c();
			byteTmpRecv.clear();
		}
		else if (itmp < 0 and byteTmpRecv.size() == 0)
		{
			//byteTmpRecv.insert(byteTmpRecv.end(), tmp, tmp + 1024);
			//什么也不做，继续滚回去循环
		}
		else
		{
			disconnect_c();
			break;
		}
	}
}

bool client::creatV4(char IP[],int port, void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)())
{
	bool test = 1;
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		test = 0;
		return test;
	}

	memset(&ADDRV4, 0, sizeof(SOCKADDR_IN));
	ADDRV4.sin_port = htons(port);
	ADDRV4.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &ADDRV4.sin_addr);

	sockClient = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	if (connect(sockClient, (SOCKADDR*)&ADDRV4, len) == SOCKET_ERROR)
	{
		test = 0;
		return test;
	}

	connect_a();

	std::thread thr_attachment_a(&client::svrEvent, this, getData_a, disconnect_a);
	thr_attachment_a.detach();

	return test;
}

bool client::creatV6(char IP[], int port, void(*getData_a)(), void(*connect_a)(), void(*disconnect_a)())
{
	bool test = 1;
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		test = 0;
		return test;
	}

	memset(&ADDRV6, 0, sizeof(SOCKADDR_IN6));
	ADDRV6.sin6_family = AF_INET6;
	ADDRV6.sin6_port = htons(port);
	inet_pton(AF_INET6, IP, &ADDRV6.sin6_addr);

	if (connect(sockClient, (SOCKADDR*)&ADDRV6, len6) == SOCKET_ERROR)
	{
		test = 0;
		return test;
	}

	std::thread thr_attachment_a(&client::svrEvent, this, getData_a, disconnect_a);
	thr_attachment_a.detach();

	return test;
}

bool client::sendCharArrToServer(int num, char* byte, int lenOfData)
{
	if ((send(sockClient, byte, lenOfData, 0)) == SOCKET_ERROR)
		return false;
	else
		return true;
}

bool client::sendStringToServer(int num, std::vector<char> byte)
{
	if ((send(sockClient, byte.data(), byte.size(), 0)) == SOCKET_ERROR)
		return false;
	else
		return true;
}


//以下为调试
std::vector<char> data;

void fk1();

void fk2()
{
	std::cout << "connect\n";
}

void fk3()
{
	std::cout << "disconnect\n";
}

int main()
{
	server server1;
	

	server1.creatV4(5123, fk1, fk2, fk3,&data);

	while (1)
		getchar();

	return 0;
}

void fk1()
{
	//server server1;
	
	char qwerty[100];
	int i=0;
	std::cout << "getdata\n" << data.size() << "\n";

	while (i < data.size())
	{
		qwerty[i] = data[i];
		i++;
	}
	qwerty[i] = '\0';

	//qwerty = tmp.data();
	printf("%s", qwerty);

}