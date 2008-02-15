#include "asserts.h"



void fail(std::string msg) {
	std::cout << "FAILED: " << msg << std::endl;
}

void pass(std::string msg) {
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



