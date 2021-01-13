#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
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

#define fourccRIFF MAKEFOURCC('R', 'I', 'F', 'F')
#define fourccDATA MAKEFOURCC('d', 'a', 't', 'a')
#define fourccFMT  MAKEFOURCC('f', 'm', 't', ' ')
#define fourccWAVE MAKEFOURCC('W', 'A', 'V', 'E')
#define fourccXWMA MAKEFOURCC('X', 'W', 'M', 'A')

	IDirectSound8* directSound = nullptr;
	IDirectSoundBuffer* primaryBuffer = nullptr;
	IDirectSoundBuffer8* secondaryBuffer = nullptr;
	BYTE* dataBuffer = nullptr;
	DWORD dataBufferSize;

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

	HRESULT FindChunk(HANDLE fileHandle, FOURCC fourcc, DWORD& chunkSize, DWORD& chunkDataPosition);

	HRESULT ReadChunkData(HANDLE fileHandle, void* buffer, DWORD buffersize, DWORD bufferoffset);

	void readWaveFile(std::wstring name);

	bool Initialise();

	void Shutdown();

	bool LoadWaveFile(TCHAR* filename);

	void ReleaseSecondaryBuffer();

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
