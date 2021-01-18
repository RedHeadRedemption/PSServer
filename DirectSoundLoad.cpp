#include "DirectSoundLoad.h"


DirectSoundLoad::DirectSoundLoad(std::wstring file) {
	
	readWaveFile(file);

}


void DirectSoundLoad::endWithError(const char* msg, int error) {
	std::cout << msg << std::endl;
	while (std::cin.get() != 10);
}


void DirectSoundLoad::readWaveFile(std::wstring name) {

	FILE* fp = NULL;
	char type[4];

	type[3] = '\0';
	const char* fileName = fromStringToChar(name);

	std::cout << fileName << std::endl;
	fp = fopen(fileName, "r");

	if (!fp) {

		endWithError("Failed to open file..");
	}

	fread(type, sizeof(char), 4, fp);

    if (strcmp(type, "RIFF")) {

		cout << "Type: " << type << endl;

	//	endWithError("Error: Not RIFF format!");
	}

	fread(&size, sizeof(DWORD), 1, fp);

	fread(type, sizeof(char), 4, fp);

	if (strcmp(type, "WAVE")) {

		cout << "Type: " << type << endl;
		//endWithError("Error: Not WAVE format!");
	}

	fread(type, sizeof(char), 4, fp);

	if (strcmp(type, "fmt")) {

		cout << "Type: " << type << endl;
		//endWithError("Error: No FMT found!");
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