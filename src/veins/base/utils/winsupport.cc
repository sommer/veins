/**
 * @file winsupport.cc
 * @brief Support for Win32 architectures.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 **/

#include "veins/base/utils/winsupport.h"

#ifdef _WIN32

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

int _asprintf(char** buf, const char* format, ...)
{
	va_list ap;
	int len;
	int res;
	assert(false); // this function is broken and it is not used right now
	va_start(ap, format);
	/* _vscprintf doesn't count the
	 * null terminating string so we add 1. */
	//len = _vscprintf_p( format, ap ) + 1; //TODO: fix error on this line when compiled with mingw
	*buf = (char*)malloc(len*sizeof(char));
	//res = _vsprintf_p(*buf, len, format, ap); //TODO: fix error on this line when compiled with mingw
	va_end(ap);

	return res;
}
#endif
