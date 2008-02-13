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
	va_list ap;
	int len;
	int res;

	va_start(ap, format);
	/* _vscprintf doesn't count the 
	 * null terminating string so we add 1. */
	len = _vscprintf_p( format, ap ) + 1;
	*buf = (char*)malloc(len*sizeof(char));
	res = _vsprintf_p(*buf, len, format, ap);
	va_end(ap);

	return res;
}

#endif
