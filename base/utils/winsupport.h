/** 
 * @file winsupport.h
 * @brief Support for Win32 architectures.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 **/
 
#ifndef __WINSUPPORT_H
#define __WINSUPPORT_H
 
#ifdef _WIN32

#include <stdarg.h>

#define __const const

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define snprintf _snprintf
#define asprintf _asprintf
 
int _asprintf(char**, const char*, ...);

#endif /* _WIN32 */
 
#endif /* __WINSUPPORT_H */
