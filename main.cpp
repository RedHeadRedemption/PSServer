#include "Connection.h"

int main()
{
	Connection c;
	HANDLE newClientThread;

	if (c.ItializeSocket() == false)
		return -1;
	else
	{
		if (c.BindSocket() == false)
			return -1;

		
		newClientThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Connection::StaticClientThread, NULL, NULL, NULL);
		if (newClientThread == NULL)
		{
			printf("There was a problem with creating a new thread.");
			return-1;
		}
	}

	WaitForSingleObject(newClientThread, INFINITE);
	return 0;
}


