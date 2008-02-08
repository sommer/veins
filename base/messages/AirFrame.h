#ifndef AIRFRAME_H_
#define AIRFRAME_H_
#include "AirFrame_m.h"

class AirFrame : public AirFrame_Base {
protected:
	/** The signal of this AirFrame */
	Signal* signal;
	
public:
	//necessary stuf when extending automatically generated message classes
	AirFrame(const char *name=NULL, int kind=0) : 
		AirFrame_Base(name,kind), signal(0) {}
	
	AirFrame(const AirFrame& other) : 
		AirFrame_Base(other.name()), signal(0) {
		
		operator=(other);
	}
	
	AirFrame& operator=(const AirFrame& other) {
		AirFrame_Base::operator=(other); 
		return *this;
	}
	
	virtual cPolymorphic *dup() const {return new AirFrame(*this);}
	
	
	
    // own extension to AirFrame	
	
	/**
	 * Returns a pointer to the Signal of this AirFrame. 
	 */
	virtual Signal* getSignalPointer();
	
	/**
	 * Sets the Signal of this AirFrame.
	 * 
	 * NOTE: The Signal has to be created by the new operator and
	 * as soon as the Signal is passed to the AirFrame the AirFrame
	 * keeps ownership of it!
	 */
	virtual void setSignal(Signal* signal_var);
	
	/**
	 * Free the signal of this AirFrame.
	 */
	virtual ~AirFrame();
	
	/**
	 * Returns a reference to the Signal of this AirFrame.
	 * 
	 * NOTE: This method is only added for compability with OMNeT.
	 * Using getSignalPointer() instead is highly recomended.
	 */
	virtual Signal& getSignal();
	
	/**
	 * Sets the signal of this AirFrame by copying the passed
	 * AirFrame.
	 * 
	 * Note: This method is mostly added for compability with OMNeT.
	 * Creating the Signal with the new operator and passing the
	 * pointer to "setSignal(Signal* signal)" is faster and therefore
	 * recommended!
	 */
    virtual void setSignal(const Signal& signal_var);
};

#endif /*AIRFRAME_H_*/
