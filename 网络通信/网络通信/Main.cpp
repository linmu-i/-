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

const int maxConnection = 256;

class server
{
private:

	std::vector<char> byteTmpRecv;

	SOCKET sockServer;
	SOCKET sockTemp[maxConnection];

	int port;
	int numOfConnect = 0;
	int numOfLastRecv;
	int numOfLastDisconnect;
	int numOfLastConnect;
	int len = sizeof(SOCKADDR_IN);
	int len6 = sizeof(SOCKADDR_IN6);

	SOCKADDR_IN ADDRV4;
	SOCKADDR_IN6 ADDRV6;

	SOCKADDR_IN clientAddr[256];

	void svrConnected(void(*getData_b)(server svr), void(*connect_b)(server svr), void(*disconnect_b)(server svr));
	void svrConnected6(void(*getData_b)(server svr), void(*connect_b)(server svr), void(*disconnect_b)(server svr));
	void svrEvent(void(*getData_c)(server svr), void(*disconnect_c)(server svr), int num);

public:

	bool creatV4(int port, void(*getData_a)(server svr), void(*connect_a)(server svr), void(*disconnect_a)(server svr));
	bool creatV6(int port, void(*getData_a)(server svr), void(*connect_a)(server svr), void(*disconnect_a)(server svr));

	bool sendCharArrToClient(int num, char* byte, int lenOfData);
	bool sendStringToClient(int num, std::vector<char> byte);

	std::vector<char> serverGetData();

	int serverGetLastRecv();
	int serverGetLastDisconnect();
	int serverGetLastConnect();

};

int server::serverGetLastRecv()
{
	return (numOfLastRecv);
}

int server::serverGetLastDisconnect()
{
	return (numOfLastDisconnect);
}

int server::serverGetLastConnect()
{
	return (numOfLastConnect);
}

std::vector<char> server::serverGetData()
{
	return(byteTmpRecv);
}

void server::svrEvent(void(*getData_c)(server svr), void(*disconnect_c)(server svr), int num)
{
	char tmp[1];
	int itmp=0;
	int i=0;
	std::vector<char> vctmp;//设置临时缓存区，并检测临时缓存区，防止检测冲突
	vctmp.clear();
	while (1)
	{
		u_long mode = 1;
		ioctlsocket(sockTemp[num], FIONBIO, &mode);
		itmp = recv(sockTemp[num], tmp, 1, 0);

		if (itmp > 0)
		{
			vctmp.insert(vctmp.end(), tmp, tmp + itmp);
		}
		else if (itmp < 0 and vctmp.size() != 0)
		{
			numOfLastRecv = num;
			byteTmpRecv = vctmp;
			getData_c(*this);
			vctmp.clear();
			byteTmpRecv.clear();
		}
		else if (itmp < 0 and vctmp.size() == 0 and (WSAGetLastError() == WSAEWOULDBLOCK))
		{
			//滚回去循环
		}
		else
		{
			numOfLastDisconnect = num;
			sockTemp[num] = 0;
			i = 0;
			disconnect_c(*this);
			while (i <= maxConnection)
			{
				if (sockTemp[i] == 0)
				{
					numOfConnect = i;
					break;
				}
				i++;
			}
			break;
		}
	}
}

void server::svrConnected(void(*getData_b)(server svr), void(*connect_b)(server svr), void(*disconnect_b)(server svr))
{
	int i;
	while (1)
	{
		if ((sockTemp[numOfConnect] = accept(sockServer, (SOCKADDR*)(&(clientAddr[numOfConnect])), &len)) != SOCKET_ERROR)
		{
			std::thread thr_attachment_b(&server::svrEvent, this, getData_b, disconnect_b, numOfConnect);
			thr_attachment_b.detach();
			numOfLastConnect = numOfConnect;
			i = 0;
			
			connect_b(*this);
			while (i <= maxConnection)
			{
				if (sockTemp[i] == 0)
				{
					numOfConnect = i;
					break;
				}
				i++;
			}
		}
	}
}

void server::svrConnected6(void(*getData_b)(server svr), void(*connect_b)(server svr), void(*disconnect_b)(server svr))
{
	int i=0;
	while (1)
	{
		if ((sockTemp[numOfConnect] = accept(sockServer, (SOCKADDR*)(&(clientAddr[numOfConnect])), &len6)) != SOCKET_ERROR)
		{
			std::thread thr_attachment_b(&server::svrEvent, this, getData_b, disconnect_b, numOfConnect);
			thr_attachment_b.detach();
			numOfLastConnect = numOfConnect;
			i = 0;

			connect_b(*this);
			while (i <= maxConnection)
			{
				if (sockTemp[i] == 0)
				{
					numOfConnect = i;
					break;
				}
				i++;
			}
		}
	}
}

bool server::creatV4(int port, void(*getData_a)(server svr), void(*connect_a)(server svr), void(*disconnect_a)(server svr))
{
	bool test = 1;
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		test = 0;
		return test;
	}

	ADDRV4.sin_family = AF_INET;
	ADDRV4.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ADDRV4.sin_port = htons(port);

	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (bind(sockServer, (sockaddr*)&ADDRV4, this->len))
	{
		test = 0;
		return test;
	}

	if (listen(sockServer, 5) == SOCKET_ERROR)
	{
		test = 0;
		return test;
	}

	std::thread thr_attachment_a(&server::svrConnected, this, getData_a, connect_a, disconnect_a);
	thr_attachment_a.detach();

	return test;
}

bool server::creatV6(int port, void(*getData_a)(server svr), void(*connect_a)(server svr), void(*disconnect_a)(server svr))
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

	std::thread thr_attachment_a(&server::svrConnected6, this, getData_a, connect_a, disconnect_a);
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

	void svrEvent(void(*getData_c)(client cli), void(*disconnect_c)(client cli));

public:

	bool creatV4(char IP[], int port, void(*getData_a)(client cli), void(*connect_a)(client cli), void(*disconnect_a)(client cli));
	bool creatV6(char IP[], int port, void(*getData_a)(client cli), void(*connect_a)(client cli), void(*disconnect_a)(client cli));

	bool sendCharArrToServer(int num, char* byte, int lenOfData);
	bool sendStringToServer(int num, std::vector<char> byte);

	std::vector<char> clientGetData();

};

std::vector<char> client::clientGetData()
{
	return (this->byteTmpRecv);
}

void client::svrEvent(void(*getData_c)(client cli), void(*disconnect_c)(client cli))
{
	char tmp[1024];
	while (1)
	{
		u_long mode = 1;
		ioctlsocket(sockClient, FIONBIO, &mode);
		int itmp = recv(sockClient, tmp, 1024, 0);

		if (itmp > 0)
		{
			byteTmpRecv.insert(byteTmpRecv.end(), tmp, tmp + itmp);
		}
		else if (itmp < 0 and byteTmpRecv.size() != 0)
		{
			getData_c(*this);
			byteTmpRecv.clear();
		}
		else if (itmp < 0 and byteTmpRecv.size() == 0 and (WSAGetLastError() == WSAEWOULDBLOCK))
		{
			//滚回去循环
		}
		else
		{
			disconnect_c(*this);
			break;
		}
	}
}

bool client::creatV4(char IP[], int port, void(*getData_a)(client cli), void(*connect_a)(client cli), void(*disconnect_a)(client cli))
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

	sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connect(sockClient, (SOCKADDR*)&ADDRV4, len) == SOCKET_ERROR)
	{
		test = 0;
		return test;
	}

	connect_a(*this);

	std::thread thr_attachment_a(&client::svrEvent, this, getData_a, disconnect_a);
	thr_attachment_a.detach();

	return test;
}

bool client::creatV6(char IP[], int port, void(*getData_a)(client cli), void(*connect_a)(client cli), void(*disconnect_a)(client cli))
{
	bool test = 1;
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		test = 0;
		return test;
	}

	sockClient = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

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

	connect_a(*this);

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



/*void fk1(server svr);

void fk2(server svr)
{
	std::cout << svr.serverGetLastConnect() << "connected\n";
}

void fk3(server svr)
{
	std::cout << "disconnected\n";
}

int main()
{
	server server1;
	char a[100];

	server1.creatV4(5123, fk1, fk2, fk3);

	while (1)
	{
		gets_s(a);
		server1.sendCharArrToClient(0, a, 99);
	}

	return 0;
}

void fk1(server svr)
{

	char *qwerty;
	int i = 0;
	std::cout << "client:" << svr.serverGetLastRecv() << "    " << svr.serverGetData().size() << "\n";

	qwerty = (char*)calloc(svr.serverGetData().size()+1, sizeof(char));

	while (i < svr.serverGetData().size())
	{
		qwerty[i] = svr.serverGetData()[i];
		i++;
	}
	qwerty[i] = '\0';

	printf("%s\n\a", qwerty);

	free(qwerty);
	
}*/