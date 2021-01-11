#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>    
#include "Server.h"

class Connection
{

public:
	static void StaticClientThread(LPVOID param);
	void ClientThread(void* param);
	bool BindSocket();
	bool ItializeSocket();
	void CloseConnection();
	void StartServer(Server * newServer);
	//void DeleteServer(Server * nextServer);
	void StopServer();

private:
	
};