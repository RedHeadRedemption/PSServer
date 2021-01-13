#pragma once
#define _CRT_SECURE_NO_WARNINGS


#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fileapi.h>
#include <fstream>

#include "DirectSoundLoad.h"

class Server
{
public:
	Server(SOCKET serverSocket);
	~Server(void);

	Server* _mainServer;
	Server* _nextServer;

	void StartStreaming();

private:
	SOCKET _clientSocket;
	static void StaticStartStreaming(LPVOID param);
	char _sendString[1024];
	std::wstring _selectedTrack;
	std::vector<std::wstring> _playList;
	void DefineUserCommands(char command);
	std::string WindowsPathToForwardSlashes();
	char* serialize(std::vector<std::string> &v, int *count);
	void deserialize(std::vector<std::string> &restore, char* buffer, int total_count);
	std::vector<std::wstring> GetTheList();
	std::vector<std::wstring> getListOfTracks(std::string folder);
	std::vector<std::string> Split(std::string stringToSplit, char delimeter);
	std::wstring getFileName(int point, std::string folder);

	bool SendData(const char * data, int totalbytes);

};
