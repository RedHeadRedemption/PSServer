#include "DirectSoundLoad.h"


DirectSoundLoad::DirectSoundLoad(std::wstring file) {
	
	readWaveFile(file);

}


void DirectSoundLoad::endWithError(const char* msg, int error) {
	std::cout << msg << std::endl;
	while (std::cin.get() != 10);
}

HRESULT DirectSoundLoad::FindChunk(HANDLE fileHandle, FOURCC fourcc, DWORD& chunkSize, DWORD& chunkDataPosition) {
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

HRESULT DirectSoundLoad::ReadChunkData(HANDLE fileHandle, void* buffer, DWORD buffersize, DWORD bufferoffset) {
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

bool DirectSoundLoad::Initialise(){
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
	bufferDesc.dwBufferBytes = 0;
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
	waveFormat.nSamplesPerSec = sampleRate;
		
		//44100;
	waveFormat.wBitsPerSample = BitsPerSample;
		
		//16;
	waveFormat.nChannels = channels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = chunkSize;

	// Set the primary buffer to be the wave format specified.
	result = primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(result))
	{
		return false;
	}
	return true;
}

void DirectSoundLoad::Shutdown() {
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

bool DirectSoundLoad::LoadWaveFile(TCHAR* filename) {
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;

	DWORD chunkSize;
	DWORD chunkPosition;
	DWORD filetype;
	HRESULT hr = S_OK;

	// Open the wave file
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	if (SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		return false;
	}
	// Make sure we have a RIFF wave file
	FindChunk(fileHandle, fourccRIFF, chunkSize, chunkPosition);
	ReadChunkData(fileHandle, &filetype, sizeof(DWORD), chunkPosition);
	if (filetype != fourccWAVE)
	{
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

	// Create a temporary sound buffer with the specific buffer settings.
	result = directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if (FAILED(result))
	{
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

void DirectSoundLoad::ReleaseSecondaryBuffer() {
	// Release the secondary sound buffer.
	if (secondaryBuffer != nullptr)
	{
		(secondaryBuffer)->Release();
		secondaryBuffer = nullptr;
	}
}

const char* DirectSoundLoad::fromStringToChar(std::wstring wideString) {

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

void DirectSoundLoad::readWaveFile(std::wstring name) {

	FILE* fp = NULL;

	const char* fileName = fromStringToChar(name);

	std::cout << fileName << std::endl;
	fp = fopen(fileName, "r");

	if (!fp) {

		endWithError("Failed to open file..");
	}

	fread(type, sizeof(char), 4, fp);

	//if (!strcmp(type, "RIFF")) {

	//	cout << type << endl;

	//	endWithError("Error: Not RIFF format!");
	//}

	fread(&size, sizeof(DWORD), 1, fp);

	fread(type, sizeof(char), 4, fp);

	if (!strcmp(type, "WAVE")) {
		endWithError("Error: Not WAVE format!");
	}

	fread(type, sizeof(char), 4, fp);

	if (!strcmp(type, "fmt")) {
		endWithError("Error: No FMT found!");
	}

	fread(&chunkSize, sizeof(DWORD), 1, fp);
	fread(&formatType, sizeof(short), 1, fp);
	fread(&channels, sizeof(short), 1, fp);
	fread(&sampleRate, sizeof(DWORD), 1, fp);
	fread(&avgBPS, sizeof(DWORD), 1, fp);
	fread(&BytesPerSample, sizeof(short), 1, fp);
	fread(&BitsPerSample, sizeof(short), 1, fp);

	fread(type, sizeof(char), 4, fp);

	if (!strcmp(type, "data")) {
		endWithError("Error: Missing Data!");
	}

	fread(&dataSize, sizeof(DWORD), 1, fp);

	std::cout << "Chunk Size:" << chunkSize << std::endl;
	std::cout << "Format Type: " << formatType << std::endl;
	std::cout << "Channels:" << channels << std::endl;
	std::cout << "Sample Rate:" << sampleRate << std::endl;
	std::cout << "Average Bytes Per Second:" << avgBPS << std::endl;
	std::cout << "Bytes Per Sample:" << BytesPerSample << std::endl;
	std::cout << "Bits Per Sample:" << BitsPerSample << std::endl;

}


DWORD DirectSoundLoad::getSize() {

	return size;
}

WORD DirectSoundLoad::getChunkSize() {

	return chunkSize;
}

DWORD DirectSoundLoad::getSampleRate() {

	return sampleRate;
}

DWORD DirectSoundLoad::getAvgBPS() {

	return avgBPS;
}

DWORD DirectSoundLoad::getDataSize() {

	return dataSize;
}

short DirectSoundLoad::getFormatType() {

	return formatType;
}

short DirectSoundLoad::getChannels() {

	return channels;
}

short DirectSoundLoad::getBytesPerSample() {

	return BytesPerSample;
}

short DirectSoundLoad::getBitsPerSample() {

	return BitsPerSample;
}