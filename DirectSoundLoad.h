#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <list>
#include <fstream>


using namespace std;

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

class DirectSoundLoad {
private:
	TCHAR* audioFilename;

	DWORD size;
	WORD chunkSize;
	short formatType, channels;
	DWORD sampleRate;
	DWORD avgBPS;
	//Bytes Per Second
	short BytesPerSample, BitsPerSample;
	DWORD dataSize;

public:
	DirectSoundLoad(std::wstring file);

	//~DirectSoundLoad();

	void readWaveFile(std::wstring name);

	void endWithError(const char* msg, int error = 0);

	const char* fromStringToChar(std::wstring wideString);

	DWORD getSize();

	WORD getChunkSize();

	DWORD getSampleRate();

	DWORD getAvgBPS();

	DWORD getDataSize();

	short getFormatType();

	short getChannels();

	short getBytesPerSample();

	short getBitsPerSample();

};
