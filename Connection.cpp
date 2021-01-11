#include "Connection.h"

SOCKET serverSocket;
int port = 5000;
Server * stoppedServer = NULL;
Server * mainServer = NULL;
HANDLE threadMutex;

// closes the socket connection
void Connection::CloseConnection()
{
	if (serverSocket == INVALID_SOCKET)
	{
		serverSocket = INVALID_SOCKET;
		closesocket(serverSocket);
	}

	if (threadMutex != NULL)
	{
		CloseHandle(threadMutex);
		threadMutex = NULL;
	}

	WSACleanup();
}

// initializes the socket and a mutex for the threads
bool Connection::ItializeSocket()
{
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 2);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "WinSock initialization has failed", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// create a mutex to allow data not being jammed when threads run concurrently
	threadMutex = CreateMutex(NULL, FALSE, NULL);
	if (threadMutex == NULL)
	{
		MessageBoxA(NULL, "An error has occured while creating a mutex", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;

}

//  start the server for a new client
void Connection::StartServer(Server * givenClient)
{
	// lock the mutex to allow the current thread to process data
	DWORD lockMutex = WaitForSingleObject(threadMutex, INFINITE);

	// if this server instance is null this means this
	// is the first client in the linked list
	if (mainServer == NULL)
	{
		mainServer = givenClient;
	}
	else
	{
		// this is not the first client in the linked list
		// so point it to the next and make that one the main
		givenClient->_nextServer = givenClient;
		mainServer = givenClient;
	}

	// unlock the mutex to allow the next thread to enter
	ReleaseMutex(threadMutex);
}

// deletes the server if a client finishes using the program
void DeleteServer(Server * givenClient)
{
	// lock the mutex to allow the current thread to process data
	DWORD lockMutex = WaitForSingleObject(threadMutex, INFINITE);

	// if this client's main is NULL it means they are
	// the last user that finished using the program
	if (givenClient->_mainServer == NULL)
	{
		// make the main server to point to this client's
		// next position in the linked list
		mainServer = givenClient->_nextServer;
	}	
	else
	{
		// this client is not the last user that uses the program
		// so move to the next empty position
		givenClient->_mainServer->_nextServer = givenClient->_nextServer;
	}

	// empty this client's position in the list
	givenClient->_mainServer = NULL;
	givenClient->_nextServer = stoppedServer;
	stoppedServer = givenClient;
	printf("Server is now closed.\n");

	// unlock the mutex to allow the next thread to enter
	ReleaseMutex(threadMutex);
}

// removes a server from the linked list
void Connection::StopServer()
{
	// lock the mutex to allow the current thread to process data
	DWORD lockMutex = WaitForSingleObject(threadMutex, INFINITE);

	// if the stopped server is not NULL there are
	// still clients that are using the program
	while (stoppedServer != NULL)
	{
		// remove the servers from clients that have
		// stopped using the program
		Server* nextServer = stoppedServer->_nextServer;
		delete stoppedServer;
		stoppedServer = nextServer;
	}

	// unlock the mutex to allow the next thread to enter
	ReleaseMutex(threadMutex);
}


// as LPTHREAD_START_ROUTINE requires a static method, 
// this method was created to solve that problem and
// leads to a non static method by reference
void Connection::StaticClientThread(LPVOID param)
{
	Connection* c = (Connection*)param;
	c->ClientThread(param);
}

// start a new client thread and run it 
// for as long as it is required
void Connection::ClientThread(void* param)
{
	while (true)
	{
		SOCKET newClientSocket = accept(serverSocket, NULL, NULL);
		if (newClientSocket == INVALID_SOCKET)
		{
			printf("Accepting the socket has failed.");
			continue;
		}

		StartServer(new Server(newClientSocket));
		StopServer();
	}
}


// binds the connection based on the given IP and Port and returns the active socket
bool Connection::BindSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("Socket failed to initialize.");
		return false;
	}

	SOCKADDR_IN addr;

	// it is suggested that sin_zero array should be filled with zeros
	// memeset is a way to fill arrays simpler similiar to the loop below
	memset(&addr, 0, sizeof(SOCKADDR_IN));

	/*for (int i = 0; i < sizeof(addr); i++)
		addr.sin_zero[i] = 0;*/

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY); 

	// bind the connection to that socket
	if (bind(serverSocket, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN)) == INVALID_SOCKET)
	{
		printf("An error had occured with bind.");
		CloseConnection();
		return false;
	}

	// start listening on the current socket
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("An error has occured with listen.");
		CloseConnection();
		return false;
	}

	printf("Server started listing on port %d...\n", port);

	return true;
	
}

