/**
 * @file winsupport.h
 * @brief Support for Win32 architectures.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 **/

#pragma once

#ifdef _WIN32

#include <stdarg.h>

#define __const const

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#endif /* _WIN32 */
