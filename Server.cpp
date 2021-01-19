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
			char recvMSG[4096];
			ZeroMemory(recvMSG, 4096);
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
				std::string hello = "\nHello from WAV streamer!\n";

				
				//char _sendString[1024] = "\nHello from WAV streamer!\n";

				std::cout << recvMSG << int(this) << std::endl;
				//send(_clientSocket, _sendString, sizeof(_sendString), 0);
				send(_clientSocket, hello.c_str(), sizeof(hello) + 1, 0);

				std::cout << sizeof(hello) << std::endl;

				//ZeroMemory(recvMSG, 32);
			}
			if (recvMSG[0] == 'L')
			{
				_playList = GetTheList();

				std::wstring serialList;

				for (int i = 0; i < _playList.size(); i++)
				{
					serialList = serialList + _playList[i] + L'|';
				}

				//serialList = serialList + L"\n";
				std::cout << recvMSG << std::endl;
				std::wcout << serialList << std::endl;

				wchar_t _sendString[4096];
				wcscpy(_sendString, serialList.c_str());
				int length = send(_clientSocket, (char*)_sendString, sizeof(_sendString) + 1, 0); // + 16
				std::cout << sizeof(_sendString) << std::endl;

				//ZeroMemory(recvMSG, 32);
			}
			if (recvMSG[0] == '1' || recvMSG[0] == '2' || recvMSG[0] == '3' || recvMSG[0] == '4' || recvMSG[0] == '5') {

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

					std::wstring name = getFileName(point, ".\\*.wav");
					fileName = name;
					//std::wstring name = getFileName(point, "C:\\Users\\User\\Documents\\Storage\\*.wav");

					DirectSoundLoad mainLoader(name);

					std::string trackInfo = "\nFile name: ";

					std::cout << trackInfo;
					std::wcout << name << endl;

					trackInfo = std::to_string(mainLoader.getChunkSize()) + '|' + std::to_string(mainLoader.getSampleRate()) + '|' + std::to_string(mainLoader.getDataSize()) + '|';
				    trackInfo = trackInfo + std::to_string(mainLoader.getChannels()) + '|' + std::to_string(mainLoader.getBitsPerSample());

					std::cout << sizeof(trackInfo) << endl;

					std::string toSend = trackInfo;

					std::cout << "Sending message:" << toSend.c_str() << std::endl;

					char _sendString[4096];
					strcpy(_sendString, toSend.c_str());
					
					//SendData(toSend.c_str(), sizeof(toSend));

					int songData = send(_clientSocket, toSend.c_str(), sizeof(toSend) + 1, 0);
				
				}
				else
				{
					std::string trackInfo = "\nSorry, this file doesn't exist in the list anymore.";
					send(_clientSocket, trackInfo.c_str(), 1024, 0);
				}
			
			}
			if (recvMSG[0] == 'P') {

				DirectSoundLoad mainLoader(fileName);

				std::wstring pathName = L".\\" + fileName;
				const char* nameIn = fromStringToChar(pathName);
				//TCHAR* extended_name = TEXT(nameIn);
				//const char* nameIn = fromStringToChar(pathName);
				char* newName = const_cast<char*>(nameIn);

				std::cout << newName << std::endl;
				bool result;

				result = Initialise(mainLoader);

					if (!result) {
						cout << "Unable to initialise DirectSound" << endl;

						std::string error = "DirectSound could not be initialized.";

						int errorSend = send(_clientSocket, error.c_str(), sizeof(error) + 1, 0);

						Shutdown();
				     }

					std::cout << "DirectSound initialization success" << std::endl;

					result = LoadWaveFile(newName);

					if (!result)
					{
						std::cout << "Unable to load wave file" << endl;

						std::string error = "Wave file could not be loaded.";

						int errorSend = send(_clientSocket, error.c_str(), sizeof(error) + 1, 0);

						Shutdown();
						
					}

					int byteSend = send(_clientSocket, reinterpret_cast<char*>(dataBuffer), sizeof(&dataBuffer), 0);

					std::cout << "Sending byte stream." << std::endl;
				
				
			}
			//delete[] recvMSG;

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
	const char* filePath = ".\\*.wav";

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

	std::string windowsPath = std::string(buffer).substr(0, fullPath);
		//+ "\\tracks\\*";

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

	const char* filePath = ".\\*.wav";

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


bool Server::Initialise(DirectSoundLoad obj) {
		HRESULT result;
		DSBUFFERDESC bufferDesc;
		WAVEFORMATEX waveFormat;

		// Initialize the direct sound interface pointer for the default sound device.
		result = DirectSoundCreate8(NULL, &directSound, NULL);
		if (FAILED(result))
		{
			return false;
		}

		// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
		// We use the handle of the desktop window since we are a console application.  If you do write a 
		// graphical application, you should use the HWnd of the graphical application. 
		result = directSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the primary buffer description.
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
		bufferDesc.dwBufferBytes = obj.getDataSize();
		bufferDesc.dwReserved = 0;
		bufferDesc.lpwfxFormat = NULL;
		bufferDesc.guid3DAlgorithm = GUID_NULL;

		// Get control of the primary sound buffer on the default sound device.
		result = directSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, NULL);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the format of the primary sound bufffer.
		// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
		// Really, we should set this up from the wave file format loaded from the file.
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nSamplesPerSec = obj.getSampleRate();

		//44100;
		waveFormat.wBitsPerSample = obj.getBitsPerSample();

		//16;
		waveFormat.nChannels = obj.getChannels();
		waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		waveFormat.cbSize = obj.getChunkSize();

		// Set the primary buffer to be the wave format specified.
		result = primaryBuffer->SetFormat(&waveFormat);
		if (FAILED(result))
		{
			return false;
		}
		return true;
	}

	void Server::Shutdown() {
		// Destroy the data buffer
		if (dataBuffer != nullptr)
		{
			delete[] dataBuffer;
			dataBuffer = nullptr;
		}

		// Release the primary sound buffer pointer.
		if (primaryBuffer != nullptr)
		{
			primaryBuffer->Release();
			primaryBuffer = nullptr;
		}

		// Release the direct sound interface pointer.
		if (directSound != nullptr)
		{
			directSound->Release();
			directSound = nullptr;
		}
	}

	bool Server::LoadWaveFile(TCHAR * filename) {
		WAVEFORMATEXTENSIBLE wfx = { 0 };
		WAVEFORMATEX waveFormat;
		DSBUFFERDESC bufferDesc;
		HRESULT result;
		IDirectSoundBuffer* tempBuffer;

		DWORD chunkSize;
		DWORD chunkPosition;
		DWORD filetype;
		HRESULT hr = S_OK;

		std::wcout << L"File Name: " << filename << std::endl;

		// Open the wave file
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			std::cout << "Unable to read file" << std::endl;
			return false;
		}
		if (SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			std::cout << "Unable to set pointer" << std::endl;
			return false;
		}
		// Make sure we have a RIFF wave file
		FindChunk(fileHandle, fourccRIFF, chunkSize, chunkPosition);
		ReadChunkData(fileHandle, &filetype, sizeof(DWORD), chunkPosition);
		if (filetype != fourccWAVE)
		{
			std::cout << "Unable to determine file type" << std::endl;
			return false;
		}
		// Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure. 
		FindChunk(fileHandle, fourccFMT, chunkSize, chunkPosition);
		ReadChunkData(fileHandle, &wfx, chunkSize, chunkPosition);
		// Find the audio data chunk
		FindChunk(fileHandle, fourccDATA, chunkSize, chunkPosition);
		dataBufferSize = chunkSize;
		// Read the audio data from the 'data' chunk.  This is the data that needs to be copied into
		// the secondary buffer for playing
		dataBuffer = new BYTE[dataBufferSize];
		ReadChunkData(fileHandle, dataBuffer, dataBufferSize, chunkPosition); // Important for transfering bytes through the socket!!
		CloseHandle(fileHandle);

		// Set the wave format of the secondary buffer that this wave file will be loaded onto.
		// The value of wfx.Format.nAvgBytesPerSec will be very useful to you since it gives you
		// an approximate value for how many bytes it takes to hold one second of audio data.
		waveFormat.wFormatTag = wfx.Format.wFormatTag;
		waveFormat.nSamplesPerSec = wfx.Format.nSamplesPerSec;
		waveFormat.wBitsPerSample = wfx.Format.wBitsPerSample;
		waveFormat.nChannels = wfx.Format.nChannels;
		waveFormat.nBlockAlign = wfx.Format.nBlockAlign;
		waveFormat.nAvgBytesPerSec = wfx.Format.nAvgBytesPerSec;
		waveFormat.cbSize = 0;

		// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto. In
		// this example, we setup a buffer the same size as that of the audio data.  For the assignment, your
		// secondary buffer should only be large enough to hold approximately four seconds of data. 
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);

		bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
		bufferDesc.dwBufferBytes = waveFormat.nAvgBytesPerSec * 4;
		//dataBufferSize;
		bufferDesc.dwReserved = 0;
		bufferDesc.lpwfxFormat = &waveFormat;
		bufferDesc.guid3DAlgorithm = GUID_NULL;

		result = directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
		if (FAILED(result))
		{
			std::cout << "Unable to create temporary buffer" << std::endl;
			return false;
		}

		// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
		result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&secondaryBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// Release the temporary buffer.
		tempBuffer->Release();
		tempBuffer = nullptr;


		return true;
	}

	void Server::ReleaseSecondaryBuffer() {
		// Release the secondary sound buffer.
		if (secondaryBuffer != nullptr)
		{
			(secondaryBuffer)->Release();
			secondaryBuffer = nullptr;
		}
	}

	const char* Server::fromStringToChar(std::wstring wideString) {

		const wchar_t* input = wideString.c_str();

		// Count required buffer size (plus one for null-terminator).
		size_t size = (wcslen(input) + 1) * sizeof(wchar_t);
		char* buffer = new char[size];

#ifdef __STDC_LIB_EXT1__
		// wcstombs_s is only guaranteed to be available if __STDC_LIB_EXT1__ is defined
		size_t convertedSize;
		std::wcstombs_s(&convertedSize, buffer, size, input, size);
#else
		std::wcstombs(buffer, input, size);
#endif

		/* Use the string stored in "buffer" variable */

		char* returnVal = buffer;
		// Free allocated memory:
		delete buffer;

		return returnVal;
	}

	HRESULT Server::FindChunk(HANDLE fileHandle, FOURCC fourcc, DWORD& chunkSize, DWORD& chunkDataPosition) {
		HRESULT hr = S_OK;
		DWORD chunkType;
		DWORD chunkDataSize;
		DWORD riffDataSize = 0;
		DWORD fileType;
		DWORD bytesRead = 0;
		DWORD offset = 0;

		if (SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		while (hr == S_OK)
		{
			if (ReadFile(fileHandle, &chunkType, sizeof(DWORD), &bytesRead, NULL) == 0)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			if (ReadFile(fileHandle, &chunkDataSize, sizeof(DWORD), &bytesRead, NULL) == 0)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			switch (chunkType)
			{
			case fourccRIFF:
				riffDataSize = chunkDataSize;
				chunkDataSize = 4;
				if (ReadFile(fileHandle, &fileType, sizeof(DWORD), &bytesRead, NULL) == 0)
				{
					hr = HRESULT_FROM_WIN32(GetLastError());
				}
				break;

			default:
				if (SetFilePointer(fileHandle, chunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
				{
					return HRESULT_FROM_WIN32(GetLastError());
				}
			}

			offset += sizeof(DWORD) * 2;
			if (chunkType == fourcc)
			{
				chunkSize = chunkDataSize;
				chunkDataPosition = offset;
				return S_OK;
			}

			offset += chunkDataSize;
			if (bytesRead >= riffDataSize)
			{
				return S_FALSE;
			}
		}
		return S_OK;
	}

HRESULT Server::ReadChunkData(HANDLE fileHandle, void* buffer, DWORD buffersize, DWORD bufferoffset) {
		HRESULT hr = S_OK;
		DWORD bytesRead;

		if (SetFilePointer(fileHandle, bufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (ReadFile(fileHandle, buffer, buffersize, &bytesRead, NULL) == 0)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		return hr;
	}
