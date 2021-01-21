#include <Windows.h>
#include "Files.h"

Files::Files() {

	fileName = NULL;
	nextFile = NULL;

}

Files::~Files() {

	if (fileName != NULL) {
		delete[] fileName;
		fileName = NULL;
	}
}