#include <windows.h>
#include "Files.h"

Files::Files(void)
{
  _fileName = NULL;
  _next = NULL;
}


Files::~Files(void)
{
  if (_fileName != NULL)
  {
	delete[] _fileName;
	_fileName = NULL;
  }
}
