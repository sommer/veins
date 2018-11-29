#include "veins_testsims/utils/asserts.h"

bool haltOnFails = false;
bool displayPassed = true;

void fail(std::string msg)
{
    EV_STATICCONTEXT;
    EV_ERROR_C("asserts") << "FAILED: " << msg << std::endl;
    if (haltOnFails) exit(1);
}

void pass(std::string msg, bool hidePassed)
{
    EV_STATICCONTEXT;
    if (!hidePassed && displayPassed) EV_INFO_C("asserts") << "Passed: " << msg << std::endl;
}

void assertTrue(std::string msg, bool value, bool hidePassed)
{
    if (!value) {
        fail(msg);
    }
    else {
        pass(msg, hidePassed);
    }
}

void assertFalse(std::string msg, bool value)
{
    assertTrue(msg, !value);
}
