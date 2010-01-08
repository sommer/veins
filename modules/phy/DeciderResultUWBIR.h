

#ifndef UWBIRDECIDERRESULT_H
#define UWBIRDECIDERRESULT_H
#include "Decider.h"
#include <vector>

class DeciderResultUWBIR : public DeciderResult {
public:
	DeciderResultUWBIR(bool isCorrect, std::vector<bool>* _decodedBits): DeciderResult(isCorrect), decodedBits(_decodedBits) { }

	~DeciderResultUWBIR() {
		decodedBits->clear();
		delete decodedBits;
	}

    // CSEM Jerome Rousselot
    const std::vector<bool>* getDecodedBits()
        { return decodedBits; }


private:
	std::vector<bool>* decodedBits; // CSEM Jerome Rousselot

};
#endif

