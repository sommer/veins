/* -*- mode:c++ -*- ********************************************************
 * file:        FWMath.h
 *
 * author:      Christian Frank
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

/**
 * @file FWMath.h
 * @author Christian Frank
 * @brief Support functions for mathematical operations.
 **/

#ifndef FWMATH_H
#define FWMATH_H

#include <cmath>

#include "MiXiMDefs.h"

#ifndef M_PI
/** @brief Windows math.h doesn't define the PI variable so we have to do it
 * by hand.*/
#define M_PI 3.14159265358979323846
#endif

#ifndef M_SQRT2
/** @brief Windows math.h doesn't define sqrt(2) so we have to do it by
 * hand. */
#define M_SQRT2 1.41421356237309504880
#endif

#ifndef EPSILON
/** @brief Constant for comparing doubles. Two doubles at most epsilon apart
 * are declared equal.*/
#define EPSILON 0.00001
#endif

/* Provide function substitutes for Win32 architectures. */
/*
#ifdef _WIN32
#ifndef _MINGW
#include <float.h>
#define finite	_finite
#define isnan	_isnan
#define erf(X)	FWMath::erf(X)
#define erfc(X)	FWMath::erfc(X)
#endif
#endif
*/

#ifdef __APPLE__
#define isnan(x) ((x) != (x))
#endif

/**
 * @brief Support functions for mathematical operations.
 *
 * This class contains all kind of mathematical support functions
 *
 * @ingroup baseUtils
 * @ingroup utils
 * @author Christian Frank
 */
class MIXIM_API FWMath
{
public:

    /**
     * Returns the rest of a whole-numbered division.
     */
    static double mod(double dividend, double divisor) {
        double i;
        return modf(dividend/divisor, &i)*divisor;
    }

    /**
     * Returns the result of a whole-numbered division.
     */
    static double div(double dividend, double divisor) {
        double i;
        /*double f;
        f=*/modf(dividend/divisor, &i);
        return i;
    }

    /**
     * @brief Returns the remainder r on division of dividend a by divisor n,
     * using floored division. The remainder r has the same sign as the divisor n.
     *
     */
    static double modulo(double a, double n) {
		return (a - n * floor(a/n));
    }

    /**
     * Tests whether two doubles are close enough to be declared equal.
     * @return true if parameters are at most epsilon apart, false
     * otherwise
     */
    static bool close(double one, double two) {
        return fabs(one-two)<EPSILON;
    }

    /**
     * @return 0 if i is close to 0, 1 if i is positive greater epsilon,
     * -1 if it is negative smaller epsilon.
     */
    static int stepfunction(double i) { return (i>EPSILON) ? 1 : close(i,0) ? 0 :-1; };


    /**
     * @return 1 if parameter greater or equal zero, -1 otherwise
     */
    static int sign(double i) { return (i>=0)? 1 : -1; };

    /**
     * @return integer that corresponds to rounded double parameter
     */
    static int round(double d) { return (int)(ceil(d-0.5)); }

    /**
     * @return the to the nearest integer towards zero rounded parameter as double
     */
    static double floorToZero(double d) { return (d >= 0.0)? floor(d) : ceil(d); }

    /**
     * @return greater of the given parameters
     */
    static double max(double a, double b) { return (a<b)? b : a; }

    /**
     * @return smaller of the given parameters
     */
    static double min(double a, double b) { return (a>b)? b : a; }

    /**
     * convert a dBm value into milli Watt
     */
    static double dBm2mW(double dBm){
        return pow(10.0, dBm/10.0);
    }

    /**
     * @brief Convert an mW value to dBm.
     **/
	static double mW2dBm(double mW) { return (10 * log10(mW)); }

    /**
     * helper function, returns squared euclidean distance
     * makes code less messy
     */
    static double torDist(double x1, double x2, double y1, double y2) {
        return (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
    }

	//TODO: resolve "extra qualification"-error for "erf" and "erfc" with mingw
	/**
	 * @brief Complementary error function.
	 **/
	static double erfc(double x) {
		double t, u, y;

		if (x <= -6.0)
			return 2.0;
		if (x >= 6.0)
			return 0.0;

		t = 3.97886080735226 / (fabs(x) + 3.97886080735226);
		u = t - 0.5;
		y = (((((((((0.00127109764952614092 * u + 1.19314022838340944e-4) * u -
			0.003963850973605135) * u - 8.70779635317295828e-4) * u +
			0.00773672528313526668) * u + 0.00383335126264887303) * u -
			0.0127223813782122755) * u - 0.0133823644533460069) * u +
			0.0161315329733252248) * u + 0.0390976845588484035) * u +
			0.00249367200053503304;
		y = ((((((((((((y * u - 0.0838864557023001992) * u -
			0.119463959964325415) * u + 0.0166207924969367356) * u +
			0.357524274449531043) * u + 0.805276408752910567) * u +
			1.18902982909273333) * u + 1.37040217682338167) * u +
			1.31314653831023098) * u + 1.07925515155856677) * u +
			0.774368199119538609) * u + 0.490165080585318424) * u +
			0.275374741597376782) * t * exp(-x * x);

		return x < 0.0 ? 2.0 - y : y;
	}

	/**
	 * @brief Error function.
	 **/
	//static double erf(double x) { return 1.0 - erfc(x); }

};

#endif
