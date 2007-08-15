/**
 * @file winmath.h
 * @brief Additional math support for Win32 architectures.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 **/

#ifndef __WINMATH_H
#define __WINMATH_H

#ifdef _WIN32

#include <math.h>
#include <float.h>
#define finite _finite
#define isnan _isnan

#define M_PI    3.14159265358979323846
#define M_SQRT2 1.41421356237309504880

double erfc(double);
double erf(double);

#endif /* _WIN32 */

#endif /* __WINMATH_H */
