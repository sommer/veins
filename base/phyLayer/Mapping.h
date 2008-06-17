#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <omnetpp.h>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <assert.h>

#include "Interpolation.h"


/**
 * @brief Specifies a dimension for functions.
 * 
 * The dimension is represented external by a string
 * (like "time") and internally by an unique id.
 */
class Dimension {
protected:
	typedef std::map<std::string, int>DimensionIDMap;
	typedef std::map<int, std::string>DimensionNameMap;
	
	/** @brief stores the registered dimensions ids.*/
	static DimensionIDMap dimensionIDs;
	
	/** @brief ConstMapping from id to name of registered dimensions.*/
	static DimensionNameMap dimensionNames;
	
	/** @brief Stores the next free ID for a new dimension.*/
	static int nextFreeID;
	
	/**
	 * @brief Returns an instance of dimension which represents
	 * the dimension with the passed name.
	 */
	static int getDimensionID(const std::string& name);
	
	/** @brief The unique id of the dimension this instance represents.*/
	int id;
	
public:
	/** @brief Shortcut to the time Dimension, same as 'Dimension("time")'.*/
	static const Dimension time;
	
public:
	Dimension():
		id(0) {}
	
	/**
	 * @brief Creates a new dimension instance representing the
	 * dimension with the passed name.
	 */
	Dimension(const std::string& name);	
	
	/**
	 * @brief Returns true if the ids of the two dimensions are equal.
	 */
	bool operator==(const Dimension& other) const;
	
	/**
	 * @brief Returns true if the id of the other dimension is
	 * greater then the id of this dimension.
	 * 
	 * This is needed to be able to use Dimension as a key in std::map.
	 */
	bool operator<(const Dimension& other) const;
	
	/**
	 * @brief Returns the name of this dimension.
	 */
	std::string getName() const{
		return dimensionNames.find(id)->second;
	}
	
	friend std::ostream& operator<<(std::ostream& out, const Dimension& d) {
    	return (out << d.getName() << "(" << d.id << ")");
    }
};

/**
 * @brief Represents a set of dimensions which is used
 * to define over which dimensions a function is defined.
 * 
 * This class actually extends from std::set<Dimension>.
 */
class DimensionSet:public std::set<Dimension> {
protected:
	typedef std::set<Dimension> DimSet;
	
	//DimensionSet& set;
public:
	typedef DimSet::const_iterator const_iterator;
public:
	/**
	 * @brief Default constructor creates an empty DimensionSet
	 */
	DimensionSet() {}
	
	/**
	 * @brief Creates a new DimensionSet with the passed Dimension as
	 * initial Dimension
	 */
	DimensionSet(const Dimension& d){		
		this->insert(d);
	}
	/**
	 * @brief Creates a new DimensionSet with the passed Dimensions as
	 * initial Dimensions (convencience method)
	 */
	DimensionSet(const Dimension& d1, const Dimension& d2){		
		this->insert(d1);
		this->insert(d2);
	}
	/**
	 * @brief Creates a new DimensionSet with the passed Dimensions as
	 * initial Dimensions (convencience method)
	 */
	DimensionSet(const Dimension& d1, const Dimension& d2, const Dimension& d3){		
		this->insert(d1);
		this->insert(d2);
		this->insert(d3);
	}
	
	/**
	 * @brief Returns the first dimension in this this set.
	 * 
	 * The first dimension is the dimension with the highest
	 * ID.
	 */
	Dimension getFirst() const{		
		return *(this->rbegin());
	}
	
	/**
	 * @brief Returns the next dimension following the passed
	 * dimension.
	 * 
	 * The next dimension is the dimension with the next lower ID.
	 * Returns the passed dimension if there is no next dimension.
	 */
	Dimension getNext(Dimension cur) const{
		if(cur == Dimension::time)
			return cur;
		
		DimSet::const_iterator it = this->find(cur);
		if(it == this->end() || it == this->begin())
			return cur;
		
		return *(--it);
	}
	
	/**
	 * @brief Returns true if the passed DimensionSet is a subset
	 * of this DimensionSet.
	 * 
	 * A DimensionSet is a subset of this DimensionSet if every
	 * Dimension of the passed DimensionSet is defined in this
	 * DimensionSet.
	 */
	bool isSubSet(const DimensionSet& other) const{
		if(this->size() < other.size())
			return false;
		
		return std::includes(this->begin(), this->end(), other.begin(), other.end());
	}
	
	/**
	 * @brief Returns true if the passed DimensionSet is a real subset
	 * of this DimensionSet.
	 * 
	 * A DimensionSet is a real subset of this DimensionSet if every
	 * Dimension of the passed DimensionSet is defined in this
	 * DimensionSet and there is at least one Dimensions in this set
	 * which isn't in the other set..
	 */
	bool isRealSubSet(const DimensionSet& other) const{
		if(this->size() <= other.size())
			return false;
		
		return std::includes(this->begin(), this->end(), other.begin(), other.end());
	}
	
	/**
	 * @brief Adds the passed dimension to the DimensionSet.
	 * 
	 * Note: "time" doesn't has to be added because it is automatically
	 * a member of every DimensionSet.
	 */
	void addDimension(const Dimension& d) {
		this->insert(d);
	}
	
	/**
	 * @brief Returns true if the passed Dimension is inside this DimensionSet.
	 */
	bool hasDimension(const Dimension& d) const{
		return this->count(d) > 0;
	}
	/*
	const_iterator begin() const{
		return this->begin();
	}
	
	const_iterator end() const{
		return this->end();
	}
	
	unsigned int size() const {
		return this->size();
	}*/
	
	/**
	 * @brief Returns true if the dimensions of both sets are equal.
	 */
	bool operator==(const DimensionSet& o){
		if(size() != o.size())
			return false;
		
		return std::equal(begin(), end(), o.begin());
	}
};

/**
 * @brief Defines an argument for a mapping.
 * 
 * Defines values for a specified set of dimensions, but at 
 * least for the time dimension.
 */
class Argument{
protected:
	
	/** @brief Stores the time dimension in Omnets time type */
	simtime_t time;
	
	typedef std::map<Dimension, double> ArgValMap;
	
	/** @brief Stores the values for the other dimensions of this argument*/
	//ArgValMap values;
	
	std::pair<Dimension, double> values[10];
	
	unsigned int count;
	
protected:
	int insertValue(int pos, const Dimension& dim, double value);
	
public:
	/**
	 * @brief Intialize this argument with the passed value for
	 * the time dimension.
	 */
	Argument(simtime_t timeVal = 0);
	
	Argument(const DimensionSet& dims, simtime_t timeVal = 0);
	
	/**
	 * @brief Returns the time value of this argument.
	 */
	simtime_t getTime() const;
	
	/**
	 * @brief Changes the time value of this argument.
	 */
	void setTime(simtime_t time);
	
	/**
	 * @brief Returns true if this Argument has a value for the
	 * passed Dimension.
	 */
	bool hasArgVal(const Dimension& dim) const;
	
	/**
	 * @brief Returns the value for the specified dimension.
	 * 
	 * Note: Don't use this function to get the time value!
	 * Use "getTime()" instead.
	 * 
	 * Returns zero if no value with the specified dimension
	 * is set for this argument.
	 */
	double getArgValue(const Dimension& dim) const;
	
	/**
	 * @brief Changes the value for the specified dimension.
	 * 
	 * Note: Don't use this function to change the time value!
	 * Use "setTime()" instead.
	 * 
	 * If the argument doesn't already contain a value for the
	 * specified dimension new dimension is added.
	 */
	void setArgValue(const Dimension& dim, double value);
	
	/**
	 * @brief Update the values of this Argument with the values
	 * of the passed Argument.
	 * 
	 * Only the dimensions from the passed Argument are updated or
	 * added.
	 */
	void setArgValues(const Argument& o);
	
	/**
	 * @brief Returns true if the passed Argument points to
	 * the same position.
	 * 
	 * The functions returns true if every Dimension in the passed
	 * Argument exists in this Argument and their values are 
	 * the same. The difference to the == operator is that the 
	 * dimensions of the passed Argument can be a subset of the
	 * dimensions of this Argument.
	 */
	bool isSamePosition(const Argument& other) const;
	
	/**
	 * @brief Two TimeArguments are compared equal if they have
	 * the same dimensions and the same values.
	 */
	bool operator==(const Argument& o) const;
	
	/**
	 * @brief Returns true if this Argument is smaller then the
	 * passed Argument. The dimensions of the Arguments have to
	 * be the same.
	 */
	bool operator<(const Argument& o) const;
	
	/**
	 * @brief Compares this Argument with the passed Argument in the
	 * dimensions of the passed DimensionsSet.
	 * 
	 * @return < 0 - passed Argument is bigger
	 * 		   = 0 - Arguments are equal
	 * 		   > 0 - passed Argument is smaller
	 */
	double compare(const Argument& o, const DimensionSet& dims) const;
	
	/**
	 * @brief Returns the dimensions this argument is defined over
	 */
	DimensionSet getDimensions() const {
		DimensionSet res(Dimension::time);
		
		for(int i = 0; i < count; i++)
			res.addDimension(values[i].first);
		
		return res;
	}
	
	friend std::ostream& operator<<(std::ostream& out, const Argument& d) {
		out << "(" << d.time;
		/*for(ArgValMap::const_iterator it = d.values.begin();
			it != d.values.end(); it++) {
			
			out << ", " << it->second;
		}*/
		for(unsigned int i = 0; i < d.count; i++){
			out << ", " << d.values[i].second;
		}
    	return (out << ")");
    }
	
	void operator=(const Argument& o);
};

/**
 * This exception is thrown by the MappingIterators when "next()" or "nextPosition()" is called
 * although "hasNext()" whould return false (means there is no next position);
 */
class NoNextIteratorExcpetion {};

/**
 * @brief Defines an const iterator for a ConstMapping which is able
 * to iterate over the Function.
 */
class ConstMappingIterator {
public:
	static const Argument& getNearestNext(ConstMappingIterator& f1, ConstMappingIterator& f2){
		const Argument& next1 = f1.getNextPosition();
		const Argument& next2 = f2.getNextPosition();
		
		if(next1 < next2)
			return next1;
		return next2;
	}
	
public:
	virtual ~ConstMappingIterator() {}
	
	/**
	 * @brief Returns the position the next call to "next()" of this
	 * Iterator whould iterate to.
	 */
	virtual const Argument& getNextPosition() const = 0;
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * The passed new position can be at arbitary places.
	 */
	virtual void jumpTo(const Argument& pos) = 0;
	
	/**
	 * @brief Lets the iterator point to the begin of the function.
	 * 
	 * The beginning of the function depends on the implementation.
	 */
	virtual void jumpToBegin() = 0;
	
	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 */
	virtual void iterateTo(const Argument& pos) = 0;
	
	/**
	 * @brief Iterates to the next position of the function.
	 * 
	 * The next position depends on the implementation of the
	 * Function.
	 */
	virtual void next() = 0;
	
	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 * 
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 */
	virtual bool inRange() const = 0;
	
	/**
	 * @brief Returns true if the iterator has a next value
	 * inside its range.
	 */
	virtual bool hasNext() const = 0;
	
	/**
	 * @brief Returns the current position of the iterator.
	 */
	virtual const Argument& getPosition() const = 0;
	
	/**
	 * @brief Returns the value of the function at the current
	 * position.
	 */
	virtual double getValue() const = 0;
};

class Mapping;

/**
 * @brief Represents a not changeable mapping (mathematical function) 
 * from at least time to double.
 * 
 * This class is an interface which describes a mapping (math.)
 * from a arbitary dimensional domain (represented by a DimensionSet) 
 * to a double value.
 */
class ConstMapping {
protected:
	DimensionSet dimensions;
public:	
	ConstMapping():
		dimensions(Dimension::time) {}
	
	ConstMapping(const DimensionSet& dimSet):
		dimensions(dimSet) {
		
		assert(dimSet.hasDimension(Dimension::time));
	}
	
	virtual ~ConstMapping() {}
	/**
	 * @brief Returns the value of this Function at possition specified
	 * by the passed Argument.
	 */
	virtual double getValue(const Argument& pos) const = 0;	
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function.
	 */
	virtual ConstMappingIterator* createConstIterator() = 0;
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function. The iterator starts at the passed position.
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) = 0;
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual ConstMapping* constClone() const = 0;
	
	/**
	 * @brief Returns the value of this Function at the possition specified
	 * by the passed Argument.
	 */
	double operator[](const Argument& pos) const {
		return getValue(pos);
	}
	
	/**
	 * @brief Returns this functions DimensionSet, which is the domain of
	 * the function.
	 */
	const DimensionSet& getDimensionSet() const {
		return dimensions;
	}
	
	/**
	 * @brief Multiplies the passed functions elementwise with each other
	 * and returns the result in a new Function.
	 * 
	 * The domain (DimensionSet) of the result is defined by the domain
	 * of the first operand.
	 * The domain of the second Mapping has to be a subset of the domain of
	 * the first mapping.
	 * 
	 * At the moment the domain of the second mapping has to be a simple subset,
	 * this means that the only missing dimensions in the second mapping have to 
	 * be the compared biggest ones. For example:
	 * 
	 * subset:
	 * domain 1 - d1, d2, d3, time
	 * domain 2 - - , d2, - , time
	 * 
	 * simple subset:
	 * domain 1 - d1, d2, d3, time
	 * domain 2 - - , - , d3, time
	 */
	static Mapping* multiply(ConstMapping& f1, ConstMapping& f2);
	
	/**
	 * @brief Multiplies the passed functions elementwise with each other
	 * and returns the result in a new Function.
	 * 
	 * The domain (DimensionSet) of the result is defined by the domain
	 * of the first operand.
	 * The domain of the second Mapping has to be a subset of the domain of
	 * the first mapping.
	 * 
	 * At the moment the domain of the second mapping has to be a simple subset,
	 * this means that the only missing dimensions in the second mapping have to 
	 * be the compared biggest ones. For example:
	 * 
	 * subset:
	 * domain 1 - d1, d2, d3, time
	 * domain 2 - - , d2, - , time
	 * 
	 * simple subset:
	 * domain 1 - d1, d2, d3, time
	 * domain 2 - - , - , d3, time
	 */
	friend Mapping* operator*(ConstMapping& f1, ConstMapping& f2) {
		return ConstMapping::multiply(f1, f2);
	}
};

/**
 * @brief A fully working ConstIterator-implementation for every ConstMapping. 
 * 
 * Although this ConstIterator whould work with almost any ConstMapping it should
 * only be used for ConstMappings whichs "getValue()"-method has constant comlexity.
 * This is because the iterator just calls the "getValue()"-method of the 
 * undelying ConstMapping on every call of itselfs "getValue()"-method.
 */
class SimpleConstMappingIterator:public ConstMappingIterator {
protected:
	ConstMapping* mapping;
	
	Argument position;
	
	typedef std::set<Argument> KeyEntrySet;
	
	const KeyEntrySet& keyEntries;
	
	KeyEntrySet::const_iterator nextEntry;
	
	DimensionSet dimensions;
	
	
public:
	/**
	 * @brief Initializes the ConstIterator for the passed ConstMapping,
	 * with the passed values for min, max and interval.
	 * 
	 * The min-Argument defines the minimum range of the domain in every 
	 * dimension and will be used as begin for the Iterator.
	 * The max-Argument defines the maximum range of the domain in every
	 * dimension and will be used as (inklusive) end for the iterator.
	 * The interval-Argument defines the stepsize used by the iterator in 
	 * every dimension.
	 */
	SimpleConstMappingIterator(ConstMapping* mapping, 
							   const std::set<Argument>& keyEntries,
							   const Argument& start):
		mapping(mapping), 
		position(start),
		keyEntries(keyEntries),
		dimensions(mapping->getDimensionSet()) {
		
		nextEntry = keyEntries.upper_bound(start);
	}
	
	/**
	 * @brief Initializes the ConstIterator for the passed ConstMapping,
	 * with the passed values for min, max and interval.
	 * 
	 * The min-Argument defines the minimum range of the domain in every 
	 * dimension and will be used as begin for the Iterator.
	 * The max-Argument defines the maximum range of the domain in every
	 * dimension and will be used as (inklusive) end for the iterator.
	 * The interval-Argument defines the stepsize used by the iterator in 
	 * every dimension.
	 */
	SimpleConstMappingIterator(ConstMapping* mapping, 
							   const std::set<Argument>& keyEntries):
		mapping(mapping), 
		keyEntries(keyEntries),
		dimensions(mapping->getDimensionSet()) {
		
		jumpToBegin();
	}
	
	virtual const Argument& getNextPosition() const { 
		if(nextEntry == keyEntries.end())
			throw NoNextIteratorExcpetion();
		
		return *nextEntry; 
	}
	
	virtual void jumpTo(const Argument& pos) { 
		position = pos;
		nextEntry = keyEntries.upper_bound(position);
	}
	
	virtual void jumpToBegin(){ 
		nextEntry = keyEntries.begin();
		position = *nextEntry;
		++nextEntry;
	}
	
	virtual void iterateTo(const Argument& pos) { 
		while(nextEntry != keyEntries.end() && !(pos < *nextEntry))
			++nextEntry;
		position = pos;
	}
	
	virtual void next() { 
		if(nextEntry == keyEntries.end())
			throw NoNextIteratorExcpetion();
		
		position = *nextEntry;
		++nextEntry;
	}
	
	virtual bool inRange() const { 
		return !(*(keyEntries.rbegin()) < position) && !(position < *(keyEntries.begin())); 
	}
	
	virtual bool hasNext() const { return nextEntry != keyEntries.end(); }
	
	virtual const Argument& getPosition() const { return position; }
	
	virtual double getValue() const { return mapping->getValue(position); }
};

/**
 * @brief Abstract subclass of ConstMapping which can be used as base for
 * any ConstMapping implementation with read access of constant complexity.
 * 
 * Any subclass only has to implement the "getValue()" and the "clone()"-method. 
 * 
 * If the SimpleMapping should be iteratable the subclass has to define
 * the range of the domain in every dimension (min and max) as well
 * as an interval which defines the steps in every dimension in which the 
 * mapping should be iterated. This should be done either at construction
 * time by calling the "SimpleConstMapping(min, max, interval)" constructor
 * or later by calling the "initializeArguments(min, max, interval)"-method.
 * If min, max and interval hasn't been initialized by one of the above
 * methods an assertion will fail when calling the CreateConstIterator()-
 * methods.
 * 
 * The SimpleConstMapping class provides Iterator creation by using the
 * SimpleConstMappingIterator which asumes that the underlying ConstMappings
 * getValue()-method is fast enough to be called on every iteration step
 * (which means constant complexity).
 */
class SimpleConstMapping:public ConstMapping {
private:
	bool fullyInitialized;
	
protected:
	typedef std::set<Argument> KeyEntrySet;
	KeyEntrySet keyEntries;
	
	friend class SimpleConstMappingIterator;
protected:
	void createKeyEntries(const Argument& from, const Argument& to, const Argument& step, Argument& pos){
			
		//get iteration borders and steps
		simtime_t fromT = from.getTime();
		simtime_t toT = to.getTime();
		simtime_t stepT = step.getTime();
		
		//iterate over interval without the end of the interval
		for(simtime_t t = fromT; (t - toT) < -0.0001; t += stepT){
			//create key entry at current position
			pos.setTime(t);
			keyEntries.insert(pos);
		}
		
		//makes sure that the end of the interval becomes it own key entry
		pos.setTime(toT);
		keyEntries.insert(pos);
	}
	
	void createKeyEntries(const Argument& from, const Argument& to, const Argument& step, 
						  DimensionSet::const_iterator curDim, Argument& pos){
		//get the dimension to iterate over
		Dimension d = *curDim;
		
		//increase iterator to next dimension (means curDim now stores the next dimension)
		--curDim;
		bool nextIsTime = (*curDim == Dimension::time);
		
		//get our iteration borders and steps
		double fromD = from.getArgValue(d);
		double toD = to.getArgValue(d);
		double stepD = step.getArgValue(d);
		
		//iterate over interval without the last entry
		for(double i = fromD; (i - toD) < -0.0001; i += stepD){
			pos.setArgValue(d, i); //update position
			
			//call iteration over sub dimension
			if(nextIsTime){
				createKeyEntries(from, to, step, pos);
			} else {
				createKeyEntries(from, to, step, curDim, pos);
			}
		}
		
		//makes sure that the end of the interval has its own key entry
		pos.setArgValue(d, toD);
		if(nextIsTime){
			createKeyEntries(from, to, step, pos);
		} else {
			createKeyEntries(from, to, step, curDim, pos);
		}
	}
		
public:	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function.
	 * 
	 * This method asserts that the min, max and interval of this mapping had been set.
	 */
	virtual ConstMappingIterator* createConstIterator() { 
		assert(fullyInitialized);
		
		return new SimpleConstMappingIterator(this, keyEntries); 
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function. The iterator starts at the passed position.
	 * 
	 * This method asserts that the min, max and interval of this mapping had been set.
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) {
		assert(fullyInitialized);
		
		return new SimpleConstMappingIterator(this, keyEntries, pos);
	}
	
public:
	SimpleConstMapping(const DimensionSet& dims):
		ConstMapping(dims), fullyInitialized(false) {}
	
	/**
	 * @brief Fully initializes this mapping with the passed min, max and interval values.
	 * 
	 * A SimpleConstMapping initialized by this constructor is able to return a valid 
	 * ConstIterator.
	 */
	SimpleConstMapping(const DimensionSet& dims, 
					   const Argument& min, const Argument& max, const Argument& interval):
		ConstMapping(dims){
		
		initializeArguments(min, max, interval);
	}
	
	/**
	 * @brief Initializes the min, max and interval-Arguments with the passed values.
	 * 
	 * After a call to this method this SimpleConstMapping is able to return a valid
	 * ConstIterator.
	 */
	void initializeArguments(const Argument& min,
							 const Argument& max,
							 const Argument& interval) {
		DimensionSet::const_iterator dimIt = dimensions.end();
		--dimIt;
		Argument pos = min;
		if(*dimIt == Dimension::time)
			createKeyEntries(min, max, interval, pos);
		else
			createKeyEntries(min, max, interval, dimIt, pos);
		
		fullyInitialized = true;
	}
	
	/**
	 * @brief This method has to be implemented by every subclass and should
	 * only have constant complexity.
	 */
	virtual double getValue(const Argument& pos) const = 0;
	
	/**
	 * @brief creates a clone of this mapping. This method has to be implemented
	 * by every subclass.
	 */
	ConstMapping* constClone() const  = 0;
};


/**
 * @brief Defines an iterator for a Mapping which is able
 * to iterate over the Function.
 */
class MappingIterator:public ConstMappingIterator {
	
public:
	virtual ~MappingIterator() {}
	/**
	 * @brief Changes the value of the function at the current 
	 * position.
	 */
	virtual void setValue(double value) = 0;
};

/**
 * @brief Represents a not changeable mapping (mathematical function) 
 * from at least time to double.
 * 
 * This class extends the ConstMapping interface with write access.
 * 
 * See ConstMapping for details.
 */
class Mapping:public ConstMapping {
public:
	enum InterpolationMethod {
		/** @brief interpolates with next lower entry*/
		STEPS, 		
		/** @brief interpolates with nearest entry*/
		NEAREST, 	
		/** @brief interpolates linear with next lower and next upper entry
				   constant before the first and after the last entry*/
		LINEAR		
	};
	
public:	
	Mapping(const DimensionSet& dims):
		ConstMapping(dims) {}
	
	Mapping():
			ConstMapping() {}
	
	virtual ~Mapping() {}
	
	/**
	 * @brief Changes the value of the function at the specified
	 * position.
	 */
	virtual void setValue(const Argument& pos, double value) = 0;
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 */
	virtual MappingIterator* createIterator() = 0;
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 * The iterator starts at the passed position.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) = 0;
	
	/**
	 * @brief Returns an ConstMappingIterator by use of the respective implementation
	 * of the "createIterator()"-method.
	 */
	virtual ConstMappingIterator* createConstIterator() {
		return createIterator();
	}
	
	/**
	 * @brief Returns an ConstMappingIterator by use of the respective implementation
	 * of the "createIterator()"-method.
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) {
		return createIterator(pos);
	}	
	
	/**
	 * @brief Returns an apropriate changeable Mapping iwth the specified domain
	 * and the specified interpolation method.
	 */
	static Mapping* createMapping(const DimensionSet& domain = DimensionSet(), 
								  InterpolationMethod intpl = STEPS);
	
	/**
	 * @brief Returns a deep copy of this Mapping.
	 */
	virtual Mapping* clone() const = 0;
	
	virtual ConstMapping* constClone() const { return clone(); }
	
};

class TimeMappingIterator:public MappingIterator {
protected:
	typedef std::map<simtime_t, double> MapType;
	typedef InterpolateableMap<simtime_t, double, 
							   Linear<simtime_t, double, 
							   		  MapType::value_type, 
							   		  MapType::const_iterator> >::intpl_iterator IteratorType;
	
	IteratorType valueIt;
	Argument position;
	Argument nextPosition;
public:
	TimeMappingIterator(IteratorType it):
		valueIt(it) {
		
		position.setTime(valueIt.getPosition());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * The passed new position can be at arbitary places.
	 */
	void jumpTo(const Argument& pos) {
		valueIt.jumpTo(pos.getTime());
		position.setTime(valueIt.getPosition());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 */
	void iterateTo(const Argument& pos) {
		valueIt.iterateTo(pos.getTime());
		position.setTime(valueIt.getPosition());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Iterates to the next position of the function.
	 * 
	 * The next position depends on the implementation of the
	 * Function.
	 */
	virtual void next() {
		valueIt.next();
		position.setTime(valueIt.getPosition());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 * 
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 */
	virtual bool inRange() const {
		return valueIt.inRange();
	}
	
	/**
	 * @brief Returns the current position of the iterator.
	 */
	virtual const Argument& getPosition() const {
		return position;
	}
	
	virtual const Argument& getNextPosition() const {
		return nextPosition;
	}
	
	/**
	 * @brief Returns the value of the function at the current
	 * position.
	 */
	virtual double getValue() const {
		return *valueIt.getValue();
	}
	
	/**
	 * @brief Lets the iterator point to the begin of the function.
	 * 
	 * The beginning of the function depends on the implementation.
	 */
	virtual void jumpToBegin() {
		valueIt.jumpToBegin();
		position.setTime(valueIt.getPosition());
	}
    
	/**
	 * @brief Returns true if the iterator has a next value
	 * inside its range.
	 */
    virtual bool hasNext() const {
    	return valueIt.hasNext();
    }
    
	/**
	 * @brief Changes the value of the function at the current 
	 * position.
	 */
	virtual void setValue(double value) {
		valueIt.setValue(value);
	}
};

/**
 * @brief Implements the WriteableTimeFunction interface by
 * a map from time to value betweeen which values can be interpolated.
 * 
 * Note: This class can only handle functions defined over time, not more.
 */
class MappedTimeFunction:public Mapping {
protected:
	typedef std::map<simtime_t, double> MapType;
	typedef InterpolateableMap<simtime_t, double, 
							   Linear<simtime_t, double,
							   		  MapType::value_type,
							   		  MapType::const_iterator> > ValueMap;
	
	/** @brief Stores the "key"-entries defining the function.*/
	ValueMap entries;
public:
	MappedTimeFunction(InterpolationMethod intpl = LINEAR):
		Mapping(), entries() {}
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const { return new MappedTimeFunction(*this); }
	
	/**
	 * @brief Returns the value of this Function at position specified
	 * by the passed Argument.
	 */
	virtual double getValue(const Argument& pos) const {
		return *entries.getIntplValue(pos.getTime());
	}
	
	/**
	 * @brief Changes the value of the function at the specified
	 * position.
	 */
	virtual void setValue(const Argument& pos, double value) {
		entries[pos.getTime()] = value;
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 */
	virtual MappingIterator* createIterator() {
		return new TimeMappingIterator(entries.beginIntpl());
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		return new TimeMappingIterator(entries.findIntpl(pos.getTime()));
	}
};

typedef MappedTimeFunction TimeMapping;

class LinearIntplMappingIterator:public MappingIterator {
protected:
	ConstMappingIterator* leftIt;
	ConstMappingIterator* rightIt;
	double factor;
	
public:
	LinearIntplMappingIterator(ConstMappingIterator* leftIt, ConstMappingIterator* rightIt, double f):
		leftIt(leftIt), rightIt(rightIt), factor(f) {
		
		assert(leftIt->getPosition() == rightIt->getPosition());
	}
	
	virtual ~LinearIntplMappingIterator() {
		if(leftIt)
			delete leftIt;
		if(rightIt)
			delete rightIt;
	}
	
	virtual bool hasNext() const { return false; }
	virtual bool inRange() const { return false; }
	
	virtual void jumpToBegin() { assert(false); }
	virtual void next() { assert(false); }
	
	virtual void setValue(double value) { assert(false); }
	
	virtual void jumpTo(const Argument& pos){
		leftIt->jumpTo(pos);
		rightIt->jumpTo(pos);
	}
	
	virtual void iterateTo(const Argument& pos){
		leftIt->iterateTo(pos);
		rightIt->iterateTo(pos);
	}
	
	virtual double getValue() const {
		double v0 = leftIt->getValue();
		double v1 = rightIt->getValue();
		return v0 + (v1 - v0) * factor;
	}
	
	virtual const Argument& getPosition() const {
		return leftIt->getPosition();
	}
	
	virtual const Argument& getNextPosition() const { assert(false); }
};

class LinearIntplMapping:public Mapping {
protected:
	ConstMapping* left;
	ConstMapping* right;
	double factor;
	
public:
	LinearIntplMapping(ConstMapping* left = 0, ConstMapping* right = 0, double f = 0.0):
		left(left), right(right), factor(f) {}
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const{ assert(false); return 0; }
	
	virtual double getValue(const Argument& pos) const {
		assert(left);
		assert(right);
		
		double v0 = left->getValue(pos);
		double v1 = right->getValue(pos);
		return v0 + (v1 - v0) * factor;
	}
	
	virtual MappingIterator* createIterator() {
		assert(false);
		return 0;
	}
	
	virtual MappingIterator* createIterator(const Argument& pos) {
		assert(left);
		assert(right);
		
		return new LinearIntplMappingIterator(left->createConstIterator(pos), right->createConstIterator(pos), factor);
	}
	
	virtual void setValue(const Argument& pos, double value) { assert(false); }
};

template<>
class Interpolated<Mapping*> {
protected:
	LinearIntplMapping mapping;
	Mapping* value;
	bool isPointer;
public:
	bool isInterpolated;
	
public:
	Interpolated(const LinearIntplMapping& m):
		mapping(m), isInterpolated(true), isPointer(false) {
		
		value = &mapping;
	}
	
	Interpolated(Mapping* m, bool isIntpl = true):
		mapping(), value(m), isInterpolated(isIntpl), isPointer(true) {}
	
	Interpolated(const Interpolated<Mapping*>& o):
		mapping() {
		
		isInterpolated = o.isInterpolated;
		isPointer = o.isPointer;
		if(isPointer){
			value = o.value;
		} else {
			mapping = o.mapping;
			value = &mapping;
		}
	}
	
	const Interpolated<Mapping*>& operator=(const Interpolated<Mapping*>& o){
		isInterpolated = o.isInterpolated;
		isPointer = o.isPointer;
		if(isPointer){
			value = o.value;
		} else {
			mapping = o.mapping;
			value = &mapping;
		}
		return *this;
	}
		
	Mapping*& operator*() {
		return value;
	}
	
	Mapping** operator->() {
		return &value;
	}
	
	bool operator==(const Interpolated<Mapping*>& other) {
		return value == other.value && isInterpolated == other.isInterpolated;
	}
	
	bool operator!=(const Interpolated<Mapping*>& other) {
		return value != other.value || isInterpolated != other.isInterpolated;
	}
};

template<>
class Linear<double, Mapping*, std::map<double, Mapping*>::value_type, std::map<double, Mapping*>::const_iterator> {
public:
	typedef std::map<double, Mapping*>::const_iterator InputIterator;
	typedef Interpolated<Mapping*> interpolated;
protected:
	
	PairLess<std::map<double, Mapping*>::value_type, double> comp;
public:
	static double linearInterpolationFactor(const double& t, 
									  const double& t0, const double& t1){
		return (t - t0) / (t1 - t0);
	}
	
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const double& pos) const{
		
		InputIterator right = std::upper_bound(first, last, pos, comp);
				
		return operator()(first, last, pos, right);
	}
	
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const double& pos, 
						 	InputIterator upperBound) const{
		if(first == last)
			return interpolated(0);
		
		if(upperBound == first)
			return interpolated(upperBound->second);
		
		InputIterator right = upperBound;
		InputIterator left = --upperBound;
		
		if(left->first == pos)
			return interpolated(left->second, false);
		
		if(right == last)
			return interpolated(left->second);
		
		double fact = linearInterpolationFactor(pos, left->first, right->first);
		
		return interpolated(LinearIntplMapping(left->second, right->second, fact));
	}
};

class MultiDimMapping;

class MultiDimMappingIterator:public MappingIterator {
protected:
	typedef InterpolateableMap<double, Mapping*, Linear<double, Mapping*, 
											     std::map<double, Mapping*>::value_type, 
											     std::map<double, Mapping*>::const_iterator> > MapType;
	typedef MapType::intpl_iterator IteratorType;
	
	IteratorType valueIt;
	MapType::interpolated subMapping;
	MappingIterator* subIterator;
	MultiDimMapping& mapping;
	Argument position;
	Argument nextPosition;
	
protected:
	void updateSubIterator(const Argument& pos) {
		MapType::interpolated subM = valueIt.getValue();
		if(subM != subMapping) {
			if(subIterator)
				delete subIterator;
			
			subMapping = subM;
			if(*subMapping)
				subIterator = (*subMapping)->createIterator(pos);
			else
				subIterator = 0;
		} else {
			if(subIterator)
				subIterator->jumpTo(pos);
		}
	}
	
	void updateSubIterator() {
		MapType::interpolated subM = valueIt.getValue();
		if(subM != subMapping) {
			if(subIterator)
				delete subIterator;
			
			subMapping = subM;
			if(*subMapping){
				if(subMapping.isInterpolated)
					subIterator = (*subMapping)->createIterator(position);
				else
					subIterator = (*subMapping)->createIterator();
			}else
				subIterator = 0;
		} else {
			if(subIterator)
				subIterator->jumpToBegin();
		}
	}
	
	void updateNextPosition();
public:
	MultiDimMappingIterator(MultiDimMapping& mapping);
	
	MultiDimMappingIterator(MultiDimMapping& mapping, const Argument& pos);
	
	virtual ~MultiDimMappingIterator() {
		if(subIterator)
			delete subIterator;
	}
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * The passed new position can be at arbitary places.
	 */
	void jumpTo(const Argument& pos);
	
	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 */
	void iterateTo(const Argument& pos);
	
	/**
	 * @brief Iterates to the next position of the function.
	 * 
	 * The next position depends on the implementation of the
	 * Function.
	 */
	virtual void next();
	
	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 * 
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 */
	virtual bool inRange() const {
		return valueIt.inRange() && (subMapping.isInterpolated || (subIterator && subIterator->inRange()));
	}
	
	/**
	 * @brief Returns the current position of the iterator.
	 */
	virtual const Argument& getPosition() const {
		return position;
	}
	
	virtual const Argument& getNextPosition() const {
		return nextPosition;
	}
	
	/**
	 * @brief Returns the value of the function at the current
	 * position.
	 */
	virtual double getValue() const {
		if(subIterator)
			return subIterator->getValue();
		else
			return 0.0;
	}
	
	/**
	 * @brief Lets the iterator point to the begin of the function.
	 * 
	 * The beginning of the function depends on the implementation.
	 */
	virtual void jumpToBegin();
    
	/**
	 * @brief Returns true if the iterator has a next value
	 * inside its range.
	 */
    virtual bool hasNext() const {
    	return valueIt.hasNext() or (subIterator and subIterator->hasNext() and valueIt.inRange());
    }
    
	/**
	 * @brief Changes the value of the function at the current 
	 * position.
	 */
	virtual void setValue(double value);
};

/**
 * @brief Implements the WriteableTimeFunction interface by
 * a map from time to value betweeen which values can be interpolated.
 * 
 * Note: This class can only handle functions defined over time, not more.
 */
class MultiDimMapping:public Mapping {
protected:
	typedef InterpolateableMap<double, Mapping*, Linear<double, Mapping*, 
											     std::map<double, Mapping*>::value_type, 
											     std::map<double, Mapping*>::const_iterator> > SubFunctionMap;
	
	/** @brief Stores the "key"-entries defining the function.*/
	SubFunctionMap entries;
	
	Dimension myDimension;
	
	friend class MultiDimMappingIterator;
	
protected:
	MultiDimMapping(const DimensionSet& myDims, Dimension myDim, InterpolationMethod intpl = STEPS):
			Mapping(myDims), entries(), myDimension(myDim) {} //TODO: implement interpolationmethod
	
public:
	MultiDimMapping(const DimensionSet& myDims, InterpolationMethod intpl = STEPS):
		Mapping(myDims), entries(), myDimension(dimensions.getFirst()) {} //TODO: implement interpolationmethod
	
	MultiDimMapping(const MultiDimMapping& o);
	
	const MultiDimMapping& operator=(const MultiDimMapping& o);
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const { return new MultiDimMapping(*this); }
	
	virtual ~MultiDimMapping() {
    	for(SubFunctionMap::iterator it = entries.begin();
    		it != entries.end(); it++) {
    		
    		if(it->second)
    			delete it->second;
    	}
    }
	
	/**
	 * @brief Returns the value of this Function at position specified
	 * by the passed Argument.
	 */
	virtual double getValue(const Argument& pos) const {
		double argVal = pos.getArgValue(myDimension);
		
		Interpolated<Mapping*> subM = entries.getIntplValue(argVal);
		
		if(!(*subM))
			return double();
		
		return (*subM)->getValue(pos);
	}
	
	/**
	 * @brief Changes the value of the function at the specified
	 * position.
	 */
	virtual void setValue(const Argument& pos, double value) {
		double argVal = pos.getArgValue(myDimension);
		
	    SubFunctionMap::iterator posIt = entries.lower_bound(argVal);
	    
		if(posIt == entries.end() || posIt->first != argVal) {
			Mapping* subF = createSubSignal();
	    	posIt = entries.insert(posIt, SubFunctionMap::value_type(argVal, subF));
	    }  
	    
	    posIt->second->setValue(pos, value);
	}
	
	Mapping* createSubSignal() const{
		Dimension nextDim = dimensions.getNext(myDimension);
		if(nextDim == Dimension::time)
			return new TimeMapping();
		else
			return new MultiDimMapping(dimensions, nextDim, LINEAR); //TODO: use interpolationmethod
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 */
	virtual MappingIterator* createIterator() {
		return new MultiDimMappingIterator(*this);
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		return new MultiDimMappingIterator(*this, pos);
	}
	
	Dimension getDimension() { return myDimension; }
};


#endif /*FUNCTION_H_*/
