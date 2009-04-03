

#ifndef UWBIRDECIDERRESULT_H
#define UWBIRDECIDERRESULT_H
#include "Decider.h"
#include <vector>

class UWBIRDeciderResult : public DeciderResult {
public:
	UWBIRDeciderResult(bool isCorrect, std::vector<bool>* _decodedBits): DeciderResult(isCorrect), decodedBits(_decodedBits) { }

	~UWBIRDeciderResult() {
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

