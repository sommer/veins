#ifndef BASE_BATTERY_H
#define BASE_BATTERY_H

#include <omnetpp.h>
class BaseUtility;

class DrainAmount {
public:
	enum PowerType {
		CURRENT,
		ENERGY
	};

protected:
	int type;
	double value;

public:
	DrainAmount(int type = CURRENT, int value = 0):
		type(type),
		value(value)
	{}

	int getType() { return type; }
	double getValue() { return value; }
	void setType(int t) { type = t; }
	void setValue(double v) { value = v; }
};

class BaseBattery : public cSimpleModule {
protected:
	/** @brief Cached pointer to the utility module*/
	BaseUtility *utility;

protected:
	cModule *findHost(void);

public:
	virtual void initialize(int stage);

	virtual int registerDevice(const std::string& name, int numAccounts) = 0;

	virtual void drain(int drainID, DrainAmount& amount, int activity) = 0;

	/** @brief Other host modules should use these interfaces to obtain
	 *  the state-of-charge of the battery.  Do NOT use BatteryState
	 *  interfaces, which should be used only by Battery Stats modules.
	 */
	/** @brief get voltage (future support for non-voltage regulated h/w */
	virtual double getVoltage() = 0;
	/** @brief current state of charge of the battery, relative to its
	 * rated nominal capacity [0..1]
	 */
	virtual double estimateResidualRelative() = 0;
	/** @brief current state of charge of the battery (mW-s) */
	virtual double estimateResidualAbs() = 0;

	/**
	 * @brief Divide initialization into two stages
	 *
	 * In the first stage (stage==0), modules subscribe to notification
	 * categories at Blackboard. The first notifications
	 * (e.g. about the initial values of some variables such as RadioState)
	 * should take place earliest in the second stage (stage==1),
	 * when everyone interested in them has already subscribed.
	 */
	virtual int numInitStages() const {
		return 2;
	}
};



#endif

