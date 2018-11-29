#pragma once

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cmath>
#include <limits>
#include <type_traits>

#include "veins/veins.h"

extern bool haltOnFails;
extern bool displayPassed;

/**
 * Prints a expect-fail message with the passed text, the passed
 * expected value and the passed actual value.
 */
template <class T, class T2>
void fail(std::string msg, T expected, T2 actual)
{
    EV_STATICCONTEXT;
    EV_ERROR_C("asserts") << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
    EV_ERROR_C("asserts") << "FAILED: " << msg << ": value was '" << actual << "' instead of '" << expected << "'" << std::endl;
}

/**
 * Prints a fail message with the passed text.
 */
void fail(std::string msg);

/**
 * Prints a pass message with the passed text.
 */
void pass(std::string msg, bool hidePassed = false);

/**
 * Asserts that the passed boolean value is true.
 */
void assertTrue(std::string msg, bool value, bool hidePassed = false);

/**
 * Asserts that the passed boolean value is false.
 */
void assertFalse(std::string msg, bool value);

/**
 * Asserts that the passed value is close to the passed expected
 * value. THis is used for floating point variables.
 */
template <class T>
void assertClose(std::string msg, T target, T actual)
{
    static_assert(std::is_floating_point<T>::value, "assertClose should only be used with floating point values");
    if (std::abs(target - actual) > 0.0000001) {
        fail(msg, target, actual);
    }
    else {
        pass(msg);
    }
}

/**
 * Asserts that the passed value is equal to the passed expected
 * value.
 */
template <class T, class T2>
void assertEqual(std::string msg, T target, T2 actual)
{
    if (target == actual) {
        pass(msg);
    }
    else {
        fail(msg, target, actual);
    }
}

/**
 * Asserts that the passed value is equal to the passed expected
 * value.
 */
template <class T, class T2>
void assertEqualSilent(std::string msg, T target, T2 actual)
{
    if (target == actual) {
        pass(msg);
    }
    else {
        fail(msg);
    }
}

/**
 * Asserts that the passed value is not equal to the passed expected
 * value.
 */
template <class T, class T2>
void assertNotEqual(std::string msg, T target, T2 actual)
{
    if (target != actual) {
        pass(msg);
    }
    else {
        fail(msg, target, actual);
    }
}

/**
 * Converts the passed value to a string. There has to be
 * an implementation of the << operator for the type of the
 * variable and std::ostream:
 *
 * std::ostream& operator<<(std::ostream& o, const T& v)
 */
template <class T>
std::string toString(const T& v)
{
    std::ostringstream o;
    o << v;
    return o.str();
};
