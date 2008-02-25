#include "asserts.h"

bool haltOnFails = false;
bool displayPassed = true;

void fail(std::string msg) {
	std::cout << "FAILED: " << msg << std::endl;
	if(haltOnFails)
		exit(1);
}

void pass(std::string msg) {
	if(displayPassed)
		std::cout << "Passed: " << msg << std::endl;
}

void assertTrue(std::string msg, bool value) {
	if (!value) {
		fail(msg);
	} else {
		pass(msg);
	}
}

void assertFalse(std::string msg, bool value) { assertTrue(msg, !value); }



