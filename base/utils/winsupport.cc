/** 
 * @file winsupport.cc
 * @brief Support for Win32 architectures.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 **/
 
#include "winsupport.h"

#ifdef _WIN32

#include <stdio.h>
#include <malloc.h>

int _asprintf(char** buf, const char* format, ...)
{
	char* p = (char*)malloc(BUF_SIZE);
	va_list ap;
	va_start(ap, format);
	int res = _vsprintf_p(p, BUF_SIZE, format, ap);
	va_end(ap);
	return res;
}

#endif
