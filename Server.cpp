#include "Connection.h"

// declaration with extern means that 
// it will be globally accessible
extern void DeleteServer(Server* s);

Server::Server(SOCKET serverSocket)
{
	_mainServer = NULL;
	_nextServer = NULL;
	_clientSocket = serverSocket;
	_sendString[1024] = NULL;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StaticStartStreaming, this, 0, NULL);

	printf("Started session.\n");
}

Server::~Server(void)
{
	if (_clientSocket == INVALID_SOCKET)
		closesocket(_clientSocket);
}

// static method required by LPTHREAD_START_ROUTINE
// that leads to a non static method by reference
void Server::StaticStartStreaming(LPVOID param)
{
	Server * server = (Server*)param;
	server->StartStreaming();
}


void Server::StartStreaming()
{
	std::cout << "Client Connected!" << std::endl;

	while (true)
	{
		try
		{
			char recvMSG[1024];
			int packetLength = recv(_clientSocket, recvMSG, sizeof(recvMSG), NULL);
			if (packetLength <= 0)
			{
				std::cout << "Error receiving command: " << WSAGetLastError() << std::endl;
				DeleteServer(this);
				break;
			}

			std::cout << "Client " << int(this) << " sent " << recvMSG[0] << std::endl;

			if (recvMSG[0] == 'H')
			{	
				char _sendString[1024] = "\nHello from WAV streamer!\n";

				std::cout << recvMSG << int(this) << std::endl;
				send(_clientSocket, _sendString, sizeof(_sendString), 0);
			}
			if (recvMSG[0] == 'L')
			{
				_playList = GetTheList();

				std::wstring serialList;

				for (int i = 0; i < _playList.size(); i++)
				{
					serialList = serialList + _playList[i] + L'|';
				}

				std::cout << recvMSG << std::endl;
				std::wcout << serialList << std::endl;
				send(_clientSocket, (char*)serialList.c_str(), sizeof(serialList) + 16, 0);
				std::cout << sizeof(serialList) << std::endl;
			}
			if (recvMSG[0] == '1' || recvMSG[0] == '2') {

				std::cout << recvMSG << std::endl;
				_playList = GetTheList();

				const size_t cSize = strlen(recvMSG) + 1;
				wchar_t* wc = new wchar_t[cSize];
				mbstowcs(wc, recvMSG, cSize);


				if (((wc == _selectedTrack) || (wc != _selectedTrack)) && (_selectedTrack != L"" || wc != L""))
					//if ( ( (recvMSG == _selectedTrack) || (recvMSG != _selectedTrack)) && (_selectedTrack != L"" || recvMSG != L"") )
				{
					std::string::size_type sz;

					_selectedTrack = wc;
					int point = std::stoi(_selectedTrack, &sz) - 1;
		
					//recvMSG;

					//Call method to get the file size in bytes and prepare it for streaming
					std::wstring name = getFileName(point, "C:\\Users\\Panayotis\\Documents\\Mysongs\\*.wav");

					std::wcout << "Opening Direct Loader" << std::endl;

					DirectSoundLoad mainLoader(name);

					std::string trackInfo = "\nFile name: ";

					std::cout << trackInfo;

					std::wcout << name << endl;

					trackInfo = "Average Bytes Per Second:" + std::to_string(mainLoader.getAvgBPS());
					
					std::cout << trackInfo << endl;

					std::string toSend = std::to_string(mainLoader.getAvgBPS());

					std::cout << "Sending message:" << toSend.c_str() << std::endl;
					
					
					//SendData(toSend.c_str(), sizeof(toSend));

					int songData = send(_clientSocket, toSend.c_str(), sizeof(toSend), 0);
				
				}
				else
				{
					std::string trackInfo = "\nSorry, this file doesn't exist in the list anymore.";
					send(_clientSocket, trackInfo.c_str(), 1024, 0);
				}
			}
			recvMSG[0] = '\0';
		}
		catch (...)
		{
			std::cout << "An error has occured." << std::endl;
			break;
		}
		
	}
	
}


bool Server::SendData(const char * data, int totalbytes)
{
	int bytesSent = 0;
	while (bytesSent < totalbytes)
	{
		int sendCheck = send(_clientSocket, data + bytesSent, totalbytes - bytesSent, NULL);
		if (sendCheck == SOCKET_ERROR)
			return false; 
		bytesSent += sendCheck;
		std::cout << "Data sent:" <<sendCheck << std::endl;
	}
	
	return true; 
}


// defines the client's command requests so that 
// the requested actions would be initatiated 
void Server::DefineUserCommands(char command)
{
	//
}
// deserializes a char buffer to a string vector array
void Server::deserialize(std::vector<std::string> &restore, char* buffer, int total_count)
{
	int temp1 = 0;
	int temp2 = 0;
	int lastNull = 0;

	//std::cout << "BUFFER " << " is: " << buffer << std::endl;

	while (lastNull != total_count)
	{
		//std::cout << "LAST NULL " << " is: " << lastNull << std::endl;
		for (int i = lastNull; i < total_count; i++) {
			const char *begin = &buffer[i];
			int size = 0;
			while (buffer[i++])
			{
				if (buffer[i] == 0)
				{
					temp1 = total_count - i;
					temp2 = (total_count - temp1);
					lastNull = temp2 + 1;
					//std::cout << "LAST NULL " << " is: " << lastNull << std::endl;
					size += 1;
					//std::cout << "ON NULL STRING IS "  << s << std::endl;
					break;
				}
				else
				{
					size += 1;
				}
			}	
			//std::cout << "STRING IS " << s << std::endl;
			restore.push_back(std::string(begin, size));
		}
	}

	//return s;
}


// serializes a vector string array to a char buffer array
char* Server::serialize(std::vector<std::string> &v, int *count)
{
	int total_count = 0;

	for (int i = 0; i < v.size(); i++) {
		//std::cout << v[i]<< std::endl;
		total_count += v[i].length() + 1;
	}

	char *buffer = new char[total_count];

	int idx = 0;

	for (int i = 0; i < v.size(); i++) {
		std::string s = v[i];
		for (int j = 0; j < s.size(); j++) {
			buffer[idx++] = s[j];
		}
		buffer[idx++] = 0;
	}

	*count = total_count;

	return buffer;
}

// Gets the contents of the music folder and returns all the wav files
std::vector<std::wstring> Server::GetTheList()
{
	std::vector<std::wstring> folderContents = getListOfTracks(WindowsPathToForwardSlashes());

	std::vector<std::wstring> trackList;

	for (int i = 0; i < folderContents.size(); i++)
	{
		if (folderContents[i].substr(folderContents[i].find_last_of(L".") + 1) == L"wav") {
			trackList.push_back(folderContents[i]);
			//std::wcout << trackList[i] << std::endl;
		}
	}

	return trackList;
}

// Retreives all the contents from the music directory
std::vector<std::wstring> Server::getListOfTracks(std::string folder)
{
	const char* filePath = "C:\\Users\\Panayotis\\Documents\\Mysongs\\*.wav";

	std::vector<std::wstring> contents;

	wchar_t* w_Path = (wchar_t*)malloc(strlen(filePath) * sizeof(wchar_t));

	mbstowcs(w_Path, filePath, strlen(filePath) + 1);

	HANDLE hFind;
	WIN32_FIND_DATAW data;

	LPCWSTR m_Path = w_Path;

	if (m_Path != 0) {

		memset(&data, 0, sizeof(WIN32_FIND_DATA));

		hFind = FindFirstFileW(m_Path, &data);

		if (hFind != INVALID_HANDLE_VALUE) {
			int i = 0;

			do {
				//printf("\n %S", data.cFileName);
				contents.push_back(data.cFileName);
				i++;
			} while (FindNextFileW(hFind, &data));
			FindClose(hFind);
		}
		else {
			std::cout << "No songs found in directory" << std::endl;
		}

		return contents;
	}
	else {
		std::cout << "Error while reading files" << std::endl;
	}
}

// Takes the full path of the current directory and changes windows' path separation style from \ to / 
std::string Server::WindowsPathToForwardSlashes()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type fullPath = std::string(buffer).find_last_of("\\/");

	std::string windowsPath = std::string(buffer).substr(0, fullPath) ; /* +"\\tracks\\*" */

	std::vector<std::string> spitList;
	std::string fixedPath;

	std::vector<std::string> splittedStrings = Split(windowsPath, '\\');

	// split windows' path
	for (int i = 0; i < splittedStrings.size(); i++)
	{
		//std::cout << splittedStrings[i] << std::endl;
		spitList.push_back(splittedStrings[i]);
	}

	// reassemble the path with forward slashes
	for (std::vector<std::string>::const_iterator i = spitList.begin(); i != spitList.end(); ++i)
	{
		fixedPath += *i + "/";
		//std::cout << fixedPath << std::endl;
	}
	fixedPath = fixedPath.substr(0, fixedPath.size() - 1);

	return fixedPath;
}

// it is used to split a string on specified delimeter - in this case to split the folder's path
std::vector<std::string> Server::Split(std::string stringToSplit, char delimeter)
{
	std::stringstream ss(stringToSplit);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}

std::wstring Server::getFileName(int point, std::string folder) {

	const char* filePath = "C:\\Users\\Panayotis\\Documents\\Mysongs\\*.wav";

	std::vector<std::wstring> contents;

	wchar_t* w_Path = (wchar_t*)malloc(strlen(filePath) * sizeof(wchar_t));

	mbstowcs(w_Path, filePath, strlen(filePath) + 1);

	HANDLE hFind;
	WIN32_FIND_DATAW data;

	LPCWSTR m_Path = w_Path;

	if (m_Path != 0) {

		memset(&data, 0, sizeof(WIN32_FIND_DATA));

		hFind = FindFirstFileW(m_Path, &data);

		if (point == 0) {

			return data.cFileName;
		}

		if (point > 0) {
			int i = 0;
			do {
				if (i == point) {
					return data.cFileName;

				}
				else {
					i++;
				}
			} while (FindNextFileW(hFind, &data));
			FindClose(hFind);
		}
		else {
			std::cout << "Index out of bounds!" << std::endl;
			return L"Index out of bounds";
		}
	}
	else {
		std::cout << "Error while reading files" << std::endl;
		return L"Error while reading files";
	}
}
