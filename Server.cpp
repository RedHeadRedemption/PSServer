#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>
#include "ServerSession.h" 


#pragma comment (lib, "ws2_32.lib")

int port = 5000;
SOCKET serverSock;

char songDir[1024];

HANDLE mutex;

ServerSession* root;
ServerSession* cRoot;

void StartS(ServerSession* session)
{
    DWORD dwWaitResult = WaitForSingleObject(mutex, INFINITE);  // no time-out interval     

    if (root == NULL) root = session;
    else
    {
        session->_child = root;
        root = session;
    }

    ReleaseMutex(mutex);
}

// close session if socket is disconnected.
void CloseS(ServerSession* session)
{
    DWORD dwWaitResult = WaitForSingleObject(mutex, INFINITE);  // no time-out interval   

    if (session->_parent == NULL) root = session->_child;
    else session->_parent->_child = session->_child;

    session->_parent = NULL;
    session->_child = cRoot;
    cRoot = session;
    printf("[%d] Ended session.\n", (DWORD)cRoot);

    ReleaseMutex(mutex);
}

// remove closed sessions from session list. 
void RemoveS()
{
    DWORD dwWaitResult = WaitForSingleObject(mutex, INFINITE);  // no time-out interval    

    while (cRoot != NULL)
    {
        ServerSession* child = cRoot->_child;
        delete cRoot;
        cRoot = child;
    }

    ReleaseMutex(mutex);
}

void Close() {
    if (serverSock != INVALID_SOCKET)
    {
        closesocket(serverSock);
        serverSock = INVALID_SOCKET;
    }
    if (mutex != NULL)
    {
        CloseHandle(mutex);
        mutex = NULL;
    }
    WSACleanup();
}

bool Initialize() {

    WSADATA w_Data;

    int iResult = WSAStartup(MAKEWORD(2, 2), &w_Data);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return false;
    }

    // create mutex to synthro
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL)
    {
        printf("CreateMutex error: %d\n", GetLastError());
        return false;
    }

    // create socket for listening.
    serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock == INVALID_SOCKET)
    {
        printf("socket failed with error: %d\n", WSAGetLastError());
        return false;
    }

    // bind listen socket.
    SOCKADDR_IN sa;
    memset(&sa, 0, sizeof(SOCKADDR_IN));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(serverSock, (LPSOCKADDR)&sa, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        Close();
        return false;
    }

    // start listen
    if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        Close();
        return false;
    }
    printf("[Server] Listening at port %d...\n", port);

    return true;
}

void serverListen(void* param)
{
    while (true)
    {
        SOCKET clientSocket = accept(serverSock, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("[Server] accept failed with error: %d\n", WSAGetLastError());
            continue;
        }
        StartS(new ServerSession(clientSocket));
        RemoveS();
    }
}


int main() {

	GetCurrentDirectory(1024, songDir);

	strcat(songDir, "\\MusicStorage\\");

	Initialize();

    HANDLE hListenThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)serverListen, NULL, 0, NULL);
    if (hListenThread == NULL)
    {
        printf("Unable to create thread.");
        Close();
        return -1;
    }

    WaitForSingleObject(hListenThread, INFINITE);
    
    Close();

    return 0;
}

