#pragma once
struct SongData
{
	int startBits;		// 0x12345678 
	int type;			// 0-status_info, 1-list_info, 2-list_block, 3-file_info, 4-file_block
	int size;
	int offset;
	int count;
	int serialnumber;
	int stopBits;		//0x87654321 
};

struct List
{
	int offset;
	int size;
};

class ServerSession {
public:
	ServerSession(SOCKET clientSocket);
	~ServerSession(void);

	ServerSession* _parent;
	ServerSession* _child;

	void Transfer();

private:
	SOCKET clientSock;
	char _recvbuf[1024];
	char _sendbuf[1024];
	SongData _response;
	char** _musicNames;
	int _musicCount;
	int _musicIndex;
	int _fileSize;

	void SetSendBuf(int type, int size = 0);
	void GetFiles();
	DWORD GetSize();
};