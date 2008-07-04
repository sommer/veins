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
 * @brief Specifies a dimension for mappings (like time, frequency, etc.)
 * 
 * The dimension is represented external by a string
 * (like "time") and internally by an unique ID.
 * 
 * Note: Since the ID for a Dimensions is set the first time an instance
 * of this dimensions is created and the id is used to provide a
 * defined ordering of the Dimensions it DOES matter which
 * dimensions are instantiated the first time.
 * Only the time dimension will always have zero as unique id.
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
	/** @brief Shortcut to the time Dimension, same as 'Dimension("time")',
	 * but spares the parsing of a string.*/
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
	
	/**
	 * @brief Returns the unique id of the dimension this instance
	 * represents.
	 * 
	 * The id is used to uniquely identify dimensions as well as to
	 * provide a sorting of dimensions.
	 * Note: The "time"-dimension will always have the ID zero.
	 */
	int getID() const { return id; }
	
	friend std::ostream& operator<<(std::ostream& out, const Dimension& d) {
    	return (out << d.getName() << "(" << d.id << ")");
    }
};

/**
 * @brief Represents a set of dimensions which is used to define over which 
 * dimensions a mapping is defined (the domain of the mapping).
 * 
 * This class actually public extends from std::set<Dimension>. So any method
 * provided by std::set can be also used.
 * 
 * The dimensions are stored ordered by their ids.
 * 
 * Note: Unlike Arguments and Mappings, a DimensionSet does not contain "time" 
 * as dimension per default. You'll have to add it like any other dimension.
 */
class DimensionSet:public std::set<Dimension> {
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
	 * initial Dimensions (convenience method)
	 */
	DimensionSet(const Dimension& d1, const Dimension& d2){		
		this->insert(d1);
		this->insert(d2);
	}
	/**
	 * @brief Creates a new DimensionSet with the passed Dimensions as
	 * initial Dimensions (convenience method)
	 */
	DimensionSet(const Dimension& d1, const Dimension& d2, const Dimension& d3){		
		this->insert(d1);
		this->insert(d2);
		this->insert(d3);
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
	 * which isn't in the other set.
	 */
	bool isRealSubSet(const DimensionSet& other) const{
		if(this->size() <= other.size())
			return false;
		
		return std::includes(this->begin(), this->end(), other.begin(), other.end());
	}
	
	/**
	 * @brief Adds the passed dimension to the DimensionSet.
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
 * 
 * Note: Currently an Argument can be maximal defined over ten Dimensions
 * plus the time dimension!
 */
class Argument{
protected:
	
	/** @brief Stores the time dimension in Omnets time type */
	simtime_t time;
	
	/** @brief Maps the dimensions of this Argument to their values. */
	std::pair<Dimension, double> values[10];
	
	/** @brief The number of Dimensions this Argument has. */
	unsigned int count;
	
protected:
	/**
	 * @brief Inserts the passed value for the passed Dimension into
	 * this Argument.
	 * 
	 * The parameter "pos" defines the position inside the Dimension<->Value-pair 
	 * array to start searching for the dimension to set.
	 * 
	 * If the "ignoreUnknown"-parameter is set to true the new value is only
	 * set if the Dimension was defined in this Argument before (means, no
	 * new DImensions are added to the Argument). 
	 * 
	 * The method returns the position inside the array the value was inserted.
	 */
	int insertValue(int pos, const Dimension& dim, double value, bool ignoreUnknown = false);
	
public:
	/**
	 * @brief Intialize this argument with the passed value for
	 * the time dimension.
	 */
	Argument(simtime_t timeVal = 0);
	
	/**
	 * @brief Initializes the Argument with the dimensions of the
	 * passed DimensionSet set to zero, and the passed value for the
	 * time (or zero, if ommited).
	 */
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
	 * specified dimension the new dimension is added.
	 */
	void setArgValue(const Dimension& dim, double value);
	
	/**
	 * @brief Update the values of this Argument with the values
	 * of the passed Argument.
	 * 
	 * Only the dimensions from the passed Argument are updated or
	 * added.
	 * 
	 * If the ignoreUnknown parameter is set to true, only the Dimensions
	 * already inside the Argument are updated.
	 */
	void setArgValues(const Argument& o, bool ignoreUnknown = false);
	
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
	 * @brief Two Arguments are compared equal if they have
	 * the same dimensions and the same values.
	 */
	bool operator==(const Argument& o) const;
	
	/**
	 * @brief Two Arguments are comapred close if they have
	 * the same dimensions and their values don't differ more
	 * then a specific epsilon.
	 */
	bool isClose(const Argument& o, double epsilon = 0.000001) const;
	
	/**
	 * @brief Returns true if this Argument is smaller then the
	 * passed Argument. The dimensions of the Arguments have to
	 * be the same.
	 * 
	 * An Argument is compared smaller than another Argument if 
	 * the value of the Dimension with the highest id is compared
	 * smaller. 
	 * If the value of the highest Dimension is compared bigger the
	 * Argument isn't compared smaller (method returns false). 
	 * If the values of the Dimension with the highest Dimension are 
	 * equal, the next smaller Dimension is compared.
	 */
	bool operator<(const Argument& o) const;
	
	/**
	 * @brief Compares this Argument with the passed Argument in the
	 * dimensions of the passed DimensionsSet. (Every other Dimension
	 * is asserted equal).
	 * 
	 * @return < 0 - passed Argument is bigger
	 * 		   = 0 - Arguments are equal
	 * 		   > 0 - passed Argument is smaller
	 * 
	 * See "operator<" for definition of smaller, equal and bigger.
	 */
	double compare(const Argument& o, const DimensionSet& dims) const;
	
	/**
	 * @brief Returns the dimensions this argument is defined over
	 * 
	 * Note: this method has linear complexity over the number of dimensions, 
	 * since the DimensionSet has to be created from the values and their 
	 * dimensions inside this Argument.
	 */
	DimensionSet getDimensions() const {
		DimensionSet res(Dimension::time);
		
		for(int i = 0; i < count; i++)
			res.addDimension(values[i].first);
		
		return res;
	}
	
	friend std::ostream& operator<<(std::ostream& out, const Argument& d) {
		out << "(" << d.time;
		
		for(unsigned int i = 0; i < d.count; i++){
			out << ", " << d.values[i].second;
		}
    	return (out << ")");
    }
	
	/**
	 * @brief Faste implementation of the copy-operator then the default 
	 * implementation.
	 */
	void operator=(const Argument& o);
};

/**
 * @brief This exception is thrown by the MappingIterators when "next()" or "nextPosition()" is called
 * although "hasNext()" whould return false (means there is no next position).
 * 
 * Although this exception isn't thrown by every implementation of the "next()"-method it is always
 * a bad idea to call "next()" although there isn't any next position.
 * You should check "hasNext()" before calling "next()" or "nextPosition()".
 */
class NoNextIteratorException {};

/**
 * @brief Defines an const iterator for a ConstMapping which is able
 * to iterate over the Mapping.
 * 
 * Iterators provide a fast way to iterate over every "entry" or important
 * position of a Mapping. They also provide constant complexity for accessing
 * the Mapping at the position of the Iterator.
 * 
 * Every implementation of an mapping-Iterator provides the same ordering
 * when iterating over a Mapping. This is that, if the iterator is increased,
 * the position of the entry it points to has to be also "increased". Which 
 * means that the new position has to be compared bigger than the previous
 * position (see class Argument for comparision of positions).
 * 
 * "Const" means that you can not change the values of the underlying Mapping
 * with this iterator. 
 */
class ConstMappingIterator {
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
	 * @brief Lets the iterator point to the begin of the mapping.
	 * 
	 * The beginning of the mapping depends on the implementation. With an
	 * implementation based on a number of key-entries, this could be the
	 * key entry with the smallest position (see class Argument for ordering
	 * of positions).
	 */
	virtual void jumpToBegin() = 0;
	
	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 * Furthermore the new position has to be compared bigger than
	 * the old position.
	 */
	virtual void iterateTo(const Argument& pos) = 0;
	
	/**
	 * @brief Iterates to the next position of the Mapping.
	 * 
	 * The next position depends on the implementation of the
	 * Mapping. With an implementation based on a number of key-entries
	 * this probably whould be the next bigger key-entry.
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
	 * @brief Returns true if the iterator has a next value it
	 * can iterate to on a call to "next()".
	 */
	virtual bool hasNext() const = 0;
	
	/**
	 * @brief Returns the current position of the iterator.
	 */
	virtual const Argument& getPosition() const = 0;
	
	/**
	 * @brief Returns the value of the Mapping at the current
	 * position.
	 * 
	 * The complexity of this method should be constant for every
	 * implementation.
	 */
	virtual double getValue() const = 0;
};

class Mapping;

/**
 * @brief Represents a not changeable mapping (mathematical function) 
 * from domain with at least the time to  a double value.
 * 
 * This class is an interface which describes a mapping (math.)
 * from a arbitary dimensional domain (represented by a DimensionSet) 
 * to a double value.
 */
class ConstMapping {
protected:
	DimensionSet dimensions;
public:	
	
	/**
	 * @brief Initializes the ConstMapping with a the time dimension as domain.
	 */
	ConstMapping():
		dimensions(Dimension::time) {}
	
	/**
	 * @brief Initializes the ConstMapping with the passed DimensionSet as
	 * Domain. 
	 * 
	 * The passed DimensionSet has to contain the time dimension!
	 */
	ConstMapping(const DimensionSet& dimSet):
		dimensions(dimSet) {
		
		assert(dimSet.hasDimension(Dimension::time));
	}
	
	virtual ~ConstMapping() {}
	
	/**
	 * @brief Returns the value of this Mapping at the position specified
	 * by the passed Argument.
	 * 
	 * The complexity of this method depends on the actual implementation.
	 */
	virtual double getValue(const Argument& pos) const = 0;	
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over this Mapping.
	 * 
	 * See class ConstIterator for details.
	 */
	virtual ConstMappingIterator* createConstIterator() = 0;
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function. The iterator starts at the passed position.
	 * 
	 * See class ConstIterator for details.
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) = 0;
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual ConstMapping* constClone() const = 0;
	
	/**
	 * @brief Returns the value of this Mapping at the position specified
	 * by the passed Argument.
	 */
	double operator[](const Argument& pos) const {
		return getValue(pos);
	}
	
	/**
	 * @brief Returns this Mappings domain as DimensionSet.
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
	 * Currently the domain of the second mapping has to be a simple subset of
	 * the domain of the first Mapping, this means that the only missing dimensions 
	 * in the second mapping have to be the compared biggest ones. 
	 * For example:
	 * 
	 * not simple subset:
	 * domain 1 - d3, d2, d1, time
	 * domain 2 - - , d2, - , time
	 *                    ^
	 *                   gap -> not a simple subset
	 * 
	 * simple subset:
	 * domain 1 - d3, d2, d1, time
	 * domain 2 - - , - , d1, time
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
	 * Currently the domain of the second mapping has to be a simple subset of
	 * the domain of the first Mapping, this means that the only missing dimensions 
	 * in the second mapping have to be the compared biggest ones. 
	 * For example:
	 * 
	 * not simple subset:
	 * domain 1 - d3, d2, d1, time
	 * domain 2 - - , d2, - , time
	 *                    ^
	 *                   gap -> not a simple subset
	 * 
	 * simple subset:
	 * domain 1 - d3, d2, d1, time
	 * domain 2 - - , - , d1, time
	 */
	friend Mapping* operator*(ConstMapping& f1, ConstMapping& f2) {
		return ConstMapping::multiply(f1, f2);
	}
};


/**
 * @brief Defines an iterator for a Mapping which is able
 * to iterate over the Mapping.
 * 
 * See class ConstMapping for details on Mapping iterators.
 * 
 * Implementations of this class are able to change the underlying
 * Mapping at the current position of the iterator with constant
 * complexity..
 */
class MappingIterator:public ConstMappingIterator {
	
public:
	virtual ~MappingIterator() {}
	/**
	 * @brief Changes the value of the Mapping at the current 
	 * position.
	 * 
	 * Implementations of this method should provide constant
	 * complexity.
	 */
	virtual void setValue(double value) = 0;
};


/**
 * @brief Represents a changeable mapping (mathematical function) 
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
	
	/**
	 * @brief Initializes the Mapping with the passed DimensionSet as domain.
	 * 
	 * The passed DimensionSet has to contain the time dimension!
	 */ 
	Mapping(const DimensionSet& dims):
		ConstMapping(dims) {}
	
	/**
	 * @brief Initializes the Mapping with the time dimension as domain.
	 */
	Mapping():
			ConstMapping() {}
	
	virtual ~Mapping() {}
	
	/**
	 * @brief Changes the value of the Mapping at the specified
	 * position.
	 * 
	 * The complexity of this method depends on the implementation.
	 */
	virtual void setValue(const Argument& pos, double value) = 0;
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the Mapping and can change it.
	 */
	virtual MappingIterator* createIterator() = 0;
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change it.
	 * 
	 * The iterator starts at the passed position.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) = 0;
	
	/**
	 * @brief Returns an ConstMappingIterator by use of the respective implementation
	 * of the "createIterator()"-method.
	 * 
	 * Override this method if your ConstIterator differs from the normal iterator.
	 */
	virtual ConstMappingIterator* createConstIterator() {
		return createIterator();
	}
	
	/**
	 * @brief Returns an ConstMappingIterator by use of the respective implementation
	 * of the "createIterator()"-method.
	 * 
	 * Override this method if your ConstIterator differs from the normal iterator.
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) {
		return createIterator(pos);
	}	
	
	/**
	 * @brief Returns an apropriate changeable Mapping with the specified domain
	 * and the specified interpolation method.
	 * 
	 * Note: The interpolation method is always linear, at the moment.
	 * 
	 * TODO: implement other interpolation methods.
	 */
	static Mapping* createMapping(const DimensionSet& domain = DimensionSet(), 
								  InterpolationMethod intpl = STEPS);
	
	/**
	 * @brief Returns a deep copy of this Mapping.
	 */
	virtual Mapping* clone() const = 0;
	
	/**
	 * @brief Returns a deep const copy of this mapping by using
	 * the according "clone()"-implementation.
	 */
	virtual ConstMapping* constClone() const { return clone(); }
	
};


//###################################################################################
//#                     default Mapping implemenations                              #
//###################################################################################

/**
 * @brief A fully working ConstIterator-implementation useable with almost every 
 * ConstMapping.
 * 
 * Although this ConstIterator whould work with almost any ConstMapping it should
 * only be used for ConstMappings whichs "getValue()"-method has constant comlexity.
 * This is because the iterator just calls the "getValue()"-method of the 
 * underlying ConstMapping on every call of itselfs "getValue()"-method.
 * 
 * The underlying ConstMapping has to provide a set of key-entries (Arguments) to the 
 * iterator to tell it the positions it should iterate over. 
 */
class SimpleConstMappingIterator:public ConstMappingIterator {
protected:
	/** @brief The underlying ConstMapping to iterate over. */
	ConstMapping* mapping;
	
	/** @brief The dimensions of the underlying Constmapping.*/
	DimensionSet dimensions;
	
	/** @brief The current position of the iterator.*/
	Argument position;
	
	typedef std::set<Argument> KeyEntrySet;
	
	/** @brief A reference to a set of Arguments defining the positions to 
	 * iterate over.
	 * TODO: change this to a pointer*/
	const KeyEntrySet& keyEntries;
	
	/** @brief An iterator over the key entry set which defines the next bigger 
	 * entry of the current position.*/
	KeyEntrySet::const_iterator nextEntry;
	
public:
	/**
	 * @brief Initializes the ConstIterator for the passed ConstMapping,
	 * with the passed key entries to iterate over and the passed position
	 * as start. 
	 * 
	 * Note: The reference to the key entries has to be valid as long as the 
	 * iterator exists.
	 */
	SimpleConstMappingIterator(ConstMapping* mapping, 
							   const std::set<Argument>& keyEntries,
							   const Argument& start);
	
	/**
	 * @brief Initializes the ConstIterator for the passed ConstMapping,
	 * with the passed key entries to iterate over.
	 * 
	 * Note: The reference to the key entries has to be valid as long as the 
	 * iterator exists.
	 */
	SimpleConstMappingIterator(ConstMapping* mapping, 
							   const std::set<Argument>& keyEntries);
	
	/**
	 * @brief Returns the next position a call to "next()" whould iterate to.
	 * 
	 * This method has constant complexity.
	 * 
	 * Throws an NoNextIteratorException if there is no next point of interest.
	 */
	virtual const Argument& getNextPosition() const { 
		if(nextEntry == keyEntries.end())
			throw NoNextIteratorException();
		
		return *nextEntry; 
	}
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * This method has logarithmic complexity (over the number of key entries). 
	 */
	virtual void jumpTo(const Argument& pos) { 
		position.setArgValues(pos, true);
		nextEntry = keyEntries.upper_bound(position);
	}
	
	/**
	 * @brief Lets the iterator point to the first "position of interest"
	 * of the underlying mapping.
	 * 
	 * This method has constant complexity.
	 */
	virtual void jumpToBegin(){ 
		nextEntry = keyEntries.begin();
		position = *nextEntry;
		++nextEntry;
	}
	
	/**
	 * @brief Increases the position of the iterator to the passed
	 * position.
	 * 
	 * The passed position has to be compared greater than the previous
	 * position.
	 * 
	 * This method has constant complexity.
	 */
	virtual void iterateTo(const Argument& pos) { 
		position.setArgValues(pos, true);
		while(nextEntry != keyEntries.end() && !(position < *nextEntry))
			++nextEntry;
		
	}
	
	/**
	 * @brief Iterates to the next "point of interest" of the Mapping.
	 * 
	 * Throws an NoNextIteratorException if there is no next point of interest.
	 * 
	 * This method has constant complexity.
	 */
	virtual void next() { 
		if(nextEntry == keyEntries.end())
			throw NoNextIteratorException();
		
		position = *nextEntry;
		++nextEntry;
	}
	
	/**
	 * @brief Returns true if the current psoition of the iterator is equal or bigger
	 * than the first point of interrest and lower or equal than the last point of
	 * interrest.
	 * 
	 * Has constant complexity.
	 */
	virtual bool inRange() const { 
		return !(*(keyEntries.rbegin()) < position) && !(position < *(keyEntries.begin())); 
	}
	
	/**
	 * @brief Returns true if there is a next position a call to "next()" can iterate to.
	 * 
	 * Has constant complexity.
	 */
	virtual bool hasNext() const { return nextEntry != keyEntries.end(); }
	
	/**
	 * @brief Returns the current position of the iterator.
	 * 
	 * Constant complexity.
	 */
	virtual const Argument& getPosition() const { return position; }
	
	/**
	 * @brief Returns the value of the underlying mapping at the current
	 * position of the iterator.
	 * 
	 * This method has the same complexity as the "getValue()" method of the
	 * underlying mapping.
	 */
	virtual double getValue() const { return mapping->getValue(position); }
};

/**
 * @brief Abstract subclass of ConstMapping which can be used as base for
 * any ConstMapping implementation with read access of constant complexity.
 * 
 * Any subclass only has to implement the "getValue()" and the "clone()"-method. 
 * 
 * If the SimpleMapping should be iterateable the subclass has to define
 * the "points of interrest" the iterator should iterate over.
 * This should be done either at construction time by calling or later by 
 * calling the "initializeArguments()"-method.
 * If the set of key entries hadn't been initialized by one of the above
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
	/** @brief Stores if the key entries had been set, and the Mapping is 
	 * therefore iterateable.*/
	bool fullyInitialized;
	
protected:
	typedef std::set<Argument> KeyEntrySet;
	
	/** @brief A set of Arguments definign the "points of interrest" an iterator
	 * should iterate over.*/
	KeyEntrySet keyEntries;
	
protected:
	
	/**
	 * @brief Utility method to fill add range of key entries in the time dimension
	 * to the key entry set.
	 */
	void createKeyEntries(const Argument& from, const Argument& to, const Argument& step, Argument& pos);
	
	/**
	 * @brief Utility method to fill add range of key entries in the passed dimension
	 * (and recursivly its sub dimensions) to the key entry set.
	 */
	void createKeyEntries(const Argument& from, const Argument& to, const Argument& step, 
						  DimensionSet::const_iterator curDim, Argument& pos);
		
public:	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the Mapping.
	 * 
	 * This method asserts that the mapping had been fully initialized.
	 */
	virtual ConstMappingIterator* createConstIterator() { 
		assert(fullyInitialized);
		
		return new SimpleConstMappingIterator(this, keyEntries); 
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the Mapping. The iterator starts at the passed position.
	 * 
	 * This method asserts that the mapping had been fully initialized.
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos) {
		assert(fullyInitialized);
		
		return new SimpleConstMappingIterator(this, keyEntries, pos);
	}
	
public:
	/**
	 * @brief Initializes a not yet iterateable SimpleConstmapping with the
	 * passed DimensionSet as domain.
	 */
	SimpleConstMapping(const DimensionSet& dims):
		ConstMapping(dims), fullyInitialized(false) {}
	
	/**
	 * @brief Fully initializes this mapping with the key entries defined by
	 * the passed min, max and interval values.
	 * 
	 * A SimpleConstMapping initialized by this constructor is able to return a valid 
	 * ConstIterator.
	 */
	SimpleConstMapping(const DimensionSet& dims, 
					   const Argument& min, const Argument& max, const Argument& interval):
		ConstMapping(dims){
		
		initializeArguments(min, max, interval);
	}
	
	//TODO: add further initialize argument methods
	
	/**
	 * @brief Initializes the key entry set with the passed min, max and 
	 * interval-Arguments.
	 * 
	 * After a call to this method this SimpleConstMapping is able to return a valid
	 * ConstIterator.
	 */
	void initializeArguments(const Argument& min,
							 const Argument& max,
							 const Argument& interval);
	
	/**
	 * @brief Returns the value of the mapping at the passed position.
	 * 
	 * This method has to be implemented by every subclass and should
	 * only have constant complexity.
	 */
	virtual double getValue(const Argument& pos) const = 0;
	
	/**
	 * @brief creates a clone of this mapping. 
	 * 
	 * This method has to be implemented by every subclass.
	 */
	ConstMapping* constClone() const  = 0;
};

/**
 * @brief Provides an implementation of the MappingIterator-
 * Interface which is able to iterate over TimeMappings.
 */
class TimeMappingIterator:public MappingIterator {
protected:
	/** @brief The std::map the InterpolateableMap is based on.*/
	typedef std::map<simtime_t, double> MapType;
	
	/** @brief The templated InterpolateableMap the underlying Mapping uses.*/
	typedef InterpolateableMap<simtime_t, double, 
							   Linear<simtime_t, double, 
							   		  MapType::value_type, 
							   		  MapType::const_iterator> >::intpl_iterator IteratorType;
	
	/** @brief Stores the current position iterator inside the Mapping.*/
	IteratorType valueIt;
	
	/** @brief Stores the current position of the iterator.*/
	Argument position;
	
	/** @brief Stores the next position a call of "next()" whould jump to.*/
	Argument nextPosition;
public:
	
	/**
	 * @brief Initializes the Iterator to use the passed InterpolateableMapIterator.
	 */
	TimeMappingIterator(IteratorType it):
		valueIt(it) {
		
		position.setTime(valueIt.getPosition());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * The passed new position can be at arbitary places.
	 * 
	 * This method has logarithmic complexity.
	 */
	void jumpTo(const Argument& pos) {
		valueIt.jumpTo(pos.getTime());
		position.setTime(pos.getTime());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 * 
	 * The passed position should compared bigger than the current position.
	 * 
	 * This method has linear complexity over the number of key-entries
	 * between the current position and the passed position. So if the 
	 * passed position is near the current position the complexity is
	 * nearly constant.
	 */
	void iterateTo(const Argument& pos) {
		valueIt.iterateTo(pos.getTime());
		position.setTime(pos.getTime());
		nextPosition.setTime(valueIt.getNextPosition());
	}
	
	/**
	 * @brief Iterates to the next position of the function.
	 * 
	 * The next position is the next bigger key entry of the 
	 * InterpoalteableMap.
	 * 
	 * This method has constant complexity.
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
	 * 
	 * THis method has cosntant complexity.
	 */
	virtual bool inRange() const {
		return valueIt.inRange();
	}
	
	/**
	 * @brief Returns the current position of the iterator.
	 * 
	 * This method has constant complexity.
	 */
	virtual const Argument& getPosition() const {
		return position;
	}
	
	/**
	 * @brief Returns the next position a call to "next()" whould jump to.
	 * 
	 * This method has constant complexity.
	 */
	virtual const Argument& getNextPosition() const {
		return nextPosition;
	}
	
	/**
	 * @brief Returns the value of the function at the current
	 * position.
	 * 
	 * This method has constant complexity.
	 */
	virtual double getValue() const {
		return *valueIt.getValue();
	}
	
	/**
	 * @brief Lets the iterator point to the begin of the mapping.
	 * 
	 * The beginning of the mapping is the smallest key entry in the
	 * InterpolateableMap.
	 * 
	 * Constant complexity.
	 */
	virtual void jumpToBegin() {
		valueIt.jumpToBegin();
		position.setTime(valueIt.getPosition());
	}
    
	/**
	 * @brief Returns true if the iterator has a next value
	 * inside its range a call to "next()" can jump to.
	 * 
	 * Constant complexity.
	 */
    virtual bool hasNext() const {
    	return valueIt.hasNext();
    }
    
	/**
	 * @brief Changes the value of the function at the current 
	 * position.
	 * 
	 * THis method has constant complexity.
	 */
	virtual void setValue(double value) {
		valueIt.setValue(value);
	}
};

/**
 * @brief Implements the Mapping-interface with an InterpolateableMap from 
 * simtime_t to double betweeen which values can be interpolated to represent
 * a Mapping with only time as domain.
 */
class MappedTimeFunction:public Mapping {
protected:
	/** @brief The type of the std::map used by the InterpolateableMap.*/
	typedef std::map<simtime_t, double> MapType;
	
	/** @brief Defines the used InterpolateableMap and its template-parameters.*/
	typedef InterpolateableMap<simtime_t, double, 
							   Linear<simtime_t, double,
							   		  MapType::value_type,
							   		  MapType::const_iterator> > ValueMap;
	
	/** @brief Stores the key-entries defining the function.*/
	ValueMap entries;
public:
	
	/**
	 * @brief Initializes the Mapping with the passed Interpolation method.
	 * 
	 * TODO: check if interpolationmethod is actually used yet.
	 */
	MappedTimeFunction(InterpolationMethod intpl = LINEAR):
		Mapping(), entries() {}
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const { return new MappedTimeFunction(*this); }
	
	/**
	 * @brief Returns the value of this Function at the position specified
	 * by the passed Argument.
	 * 
	 * This method has logarithmic complexity.
	 */
	virtual double getValue(const Argument& pos) const {
		return *entries.getIntplValue(pos.getTime());
	}
	
	/**
	 * @brief Changes the value of the function at the specified
	 * position.
	 * 
	 * This method has logarithmic complexity.
	 */
	virtual void setValue(const Argument& pos, double value) {
		entries[pos.getTime()] = value;
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 * 
	 * Note: The caller of this method has to delete the returned Iterator
	 * pointer if it isn't used anymore.
	 */
	virtual MappingIterator* createIterator() {
		return new TimeMappingIterator(entries.beginIntpl());
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 * 
	 * Note: The caller of this method has to delete the returned Iterator
	 * pointer if it isn't used anymore.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		return new TimeMappingIterator(entries.findIntpl(pos.getTime()));
	}
};

typedef MappedTimeFunction TimeMapping;


/**
 * @brief Helperclass for the MultiDimMapping which provides an Iterator 
 * which linear interpolates between two other Mapping iterators. Or in
 * other words, it provides an Interator for an linear interpolated Mapping.
 */
class LinearIntplMappingIterator:public MappingIterator {
protected:
	/** @brief Iterator for the left Mapping to interpolate.*/
	ConstMappingIterator* leftIt;
	/** @brief Iterator for the right Mapping to interpolate.*/
	ConstMappingIterator* rightIt;
	
	/** @brief The factor defining how strong the left and the right Mapping 
	 * affect the interpoaltion.*/
	double factor;
	
public:
	/**
	 * @brief Initializes the Interator with the passed Iterators of the mappings to 
	 * interpoalte and the their inteproaltionfactor.
	 */
	LinearIntplMappingIterator(ConstMappingIterator* leftIt, ConstMappingIterator* rightIt, double f);
	
	/**
	 * @brief Deletes the left and the right mapping iterator.
	 */
	virtual ~LinearIntplMappingIterator();
	
	/**
	 * @brief An interpolated mapping isn't really iterateable over specific
	 * values, it only provides an fast way to get several values from an
	 * Interpoalted mapping.
	 */
	virtual bool hasNext() const { return false; }
	/**
	 * @brief An interpolated mapping isn't really iterateable over specific
	 * values, it only provides an fast way to get several values from an
	 * Interpoalted mapping.
	 */
	virtual bool inRange() const { return false; }
	
	/**
	 * @brief This method isn't supported by an interpolated Mapping.
	 */
	virtual void jumpToBegin() { assert(false); }
	/**
	 * @brief This method isn't supported by an interpolated Mapping.
	 */
	virtual void next() { assert(false); }
	
	/**
	 * @brief This method isn't supported by an interpolated Mapping.
	 */
	virtual void setValue(double value) { assert(false); }
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * This method has logarithmic complexity over both of the
	 * underlying Mappings used to interpolate.
	 */
	virtual void jumpTo(const Argument& pos){
		leftIt->jumpTo(pos);
		rightIt->jumpTo(pos);
	}
	
	/**
	 * @brief Increases the iterator to the passed position. This
	 * position should be near the current position of the iterator.
	 * 
	 * The passed position has to be compared bigger than the current position.
	 * 
	 * This method has linear complexity over the number of key entries between
	 * the current and the passed position in both underlying mappings used to 
	 * interpolate. So if the passed position is near the current position this
	 * method has nearly constant complexity.
	 */
	virtual void iterateTo(const Argument& pos){
		leftIt->iterateTo(pos);
		rightIt->iterateTo(pos);
	}
	
	/**
	 * @brief Returns the value of the Interpolated mapping at the current 
	 * position of the iterator.
	 * 
	 * This method has constant complexity.
	 */
	virtual double getValue() const {
		double v0 = leftIt->getValue();
		double v1 = rightIt->getValue();
		return v0 + (v1 - v0) * factor;
	}
	
	/**
	 * @brief Returns the current position of the iterator.
	 * 
	 * Constant complexity.
	 */
	virtual const Argument& getPosition() const {
		return leftIt->getPosition();
	}
	
	/**
	 * @brief This method isn't supported by an interpolated mapping.
	 */
	virtual const Argument& getNextPosition() const { assert(false); }
};

/**
 * @brief Helperclass which represents a linear interpolation between
 * two other mappings.
 */
class LinearIntplMapping:public Mapping {
protected:
	/** @brief The left mapping to interpolate.*/
	ConstMapping* left;
	/** @brief The right mapping to interpolate*/
	ConstMapping* right;
	
	/** @brief The interpolationfactor determining the linear interpolation
	 * between left and right mapping.*/
	double factor;
	
public:
	
	/**
	 * @brief Initializes the LinearIntplMapping with the passed left and right
	 * Mapping to interpoalte by the passed interpolation value.
	 */
	LinearIntplMapping(ConstMapping* left = 0, ConstMapping* right = 0, double f = 0.0):
		left(left), right(right), factor(f) {}
	
	/**
	 * @brief Interpolated mappings are not supposed to be cloned!
	 */
	virtual Mapping* clone() const{ assert(false); return 0; }
	
	/**
	 * @brief Returns the linear interpolated value of this Mapping.
	 * 
	 * The value is caclulated by the following formula:
	 * 
	 * v = left + (right - left) * intplFactor
	 */
	virtual double getValue(const Argument& pos) const {
		assert(left);
		assert(right);
		
		double v0 = left->getValue(pos);
		double v1 = right->getValue(pos);
		return v0 + (v1 - v0) * factor;
	}
	
	/**
	 * @brief An interpolated mapping doesn't have a valid "first"-entry,
	 * so this method is not supported.
	 */
	virtual MappingIterator* createIterator() {
		assert(false);
		return 0;
	}
	
	/**
	 * @brief Creates an iterator for this mapping starting at
	 * the passed position.
	 * 
	 * Note: The returned Iterator does only support a subset of the
	 * normal MappingIterator-methods. See LinearIntplMappingIterator for
	 * details.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		assert(left);
		assert(right);
		
		return new LinearIntplMappingIterator(left->createConstIterator(pos), right->createConstIterator(pos), factor);
	}
	
	/**
	 * @brief This method is not supported!
	 */
	virtual void setValue(const Argument& pos, double value) { assert(false); }
};

/**
 * @brief Helperclass (-specialisation) for multiDimMapping which is used by an
 * InterpolateableMap as return value of the "getValue()" - method.
 * 
 * Provides either an pointer to an actual SubMapping of the MultiDimMapping or
 * a Pointer to an temporary InterpolatedMapping between two Submappings of the
 * MultiDimMapping.
 */
template<>
class Interpolated<Mapping*> {
protected:
	/** @brief Holds the temporary InterpolatedMapping if neccessary.*/
	LinearIntplMapping mapping;
	
	/** @brief A pointer to the Mapping this class represents.*/
	Mapping* value;
	
	/** @brief Stores if we use the temporary IntplMapping or a extern pointer.*/
	bool isPointer;
public:
	/** @brief Stores if the underlying Mapping is interpolated or not.*/
	bool isInterpolated;
	
public:
	/**
	 * @brief Initializes this Interpolated instance to represent the passed
	 * Interpoalted Mapping. Copies the passed Mapping to its internal member.
	 * Sets "isInterpolated" to true.
	 */
	Interpolated(const LinearIntplMapping& m):
		mapping(m), isInterpolated(true), isPointer(false) {
		
		value = &mapping;
	}
	
	/**
	 * @brief Initializes this Interpoalted instance to represent the Mapping
	 * the passed pointer points to and with the passed isIntpl value.
	 * 
	 * The passed pointer has to be valid as long as this instance exists.
	 */
	Interpolated(Mapping* m, bool isIntpl = true):
		mapping(), value(m), isInterpolated(isIntpl), isPointer(true) {}
	
	/**
	 * @brief Copyconstructor which assures that the internal storage is used correctly.
	 */
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
	
	/**
	 * @brief Assignment operator which assures that the internal storage is copied
	 * correctly.
	 */
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
		
	/**
	 * @brief Dereferences this Interpoalted to the represented value (works like
	 * dereferencing an std::iterator).
	 */
	Mapping*& operator*() {
		return value;
	}
	
	/**
	 * @brief Dereferences this Interpoalted to the represented value (works like
	 * dereferencing an std::iterator).
	 */
	Mapping** operator->() {
		return &value;
	}
	
	/**
	 * @brief Two Interpoalted<Mapping*> are compared equal if the pointer to
	 * the represented Mapping is the same as well as the "isInterpolated"
	 * value.
	 */
	bool operator==(const Interpolated<Mapping*>& other) {
		return value == other.value && isInterpolated == other.isInterpolated;
	}
	
	/**
	 * @brief Two Interpoalted<Mapping*> are compared non equal if the pointer to
	 * the represented Mapping differs or the "isInterpolated"
	 * value.
	 */
	bool operator!=(const Interpolated<Mapping*>& other) {
		return value != other.value || isInterpolated != other.isInterpolated;
	}
};

/**
 * @brief Specialisation of the Linear-template which provides LinearInterpolation 
 * for pointer two Mappings. Used by MultiDimMapping.
 */
template<>
class Linear<double, Mapping*, std::map<double, Mapping*>::value_type, std::map<double, Mapping*>::const_iterator> {
public:
	/** @brief Iterator type of the mapping over which to interpolate.*/
	typedef std::map<double, Mapping*>::const_iterator InputIterator;
	
	/** @brief Interpolated type used as return value.*/
	typedef Interpolated<Mapping*> interpolated;
protected:
	
	/** @brief Comparission class for the values of the map to interpolate over.*/
	PairLess<std::map<double, Mapping*>::value_type, double> comp;
public:
	
	/**
	 * @brief calculates the linear interpoaltion factor used for the created
	 * LinearIntplMappings.
	 */
	static double linearInterpolationFactor(const double& t, 
									  const double& t0, const double& t1){
		return (t - t0) / (t1 - t0);
	}
	
	/**
	 * @brief Functor operator of this class which linear interpolates the value
	 * at the passed position using the values between the passed Iterators.
	 * 
	 * The returned instance of interpolated represents the result. Which can be
	 * either an actual entry of the interpoalted map (if the position two
	 * interpolate was exactly that. Or it can be an interpolated value, if the 
	 * passed position was between two entries of the map.
	 * This state can be retrieved with the "isInterpolated"-Member of the returned
	 * "interpolated".
	 */
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const double& pos) const{
		
		InputIterator right = std::upper_bound(first, last, pos, comp);
				
		return operator()(first, last, pos, right);
	}
	
	/**
	 * @brief Functor operator of this class which linear interpolates the value
	 * at the passed position using the values between the passed Iterators.
	 * 
	 * The upperBound-iterator has to point two the entry next bigger as the
	 * passed position to interpolate.
	 * 
	 * The returned instance of interpolated represents the result. Which can be
	 * either an actual entry of the interpoalted map (if the position two
	 * interpolate was exactly that. Or it can be an interpolated value, if the 
	 * passed position was between two entries of the map.
	 * This state can be retrieved with the "isInterpolated"-Member of the returned
	 * "interpolated".
	 */
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

/**
 * @brief Implementation of the MappingIterator-interface which is able 
 * to iterate over every value in a MultiDimMapping.
 * 
 * As the MultiDimMapping has a treelike structure of submappings to
 * represent multiple dimensions, the MultiDimIterator consist of a
 * number of sub-MultiDimIterator to represent the current position
 * inside the submappings. So every submapping-iterator represents
 * one dimension and the and Iterator to next Dimensions.
 * The last iterator is an TimeMappingIterator.
 * 
 * Iteration works by sub-iterator-first-iteration. WHich means that
 * at first the sub-iterator at the current position is iterated to its
 * end before the position inside the dimension of this iterator is increased.
 * This assures the iterationorder demanded by the MappingIterator-interface.
 */
class MultiDimMappingIterator:public MappingIterator {
protected:
	/** @brief The type of the InterpolateableMap used by the underlying Mapping.*/
	typedef InterpolateableMap<double, Mapping*, Linear<double, Mapping*, 
											     std::map<double, Mapping*>::value_type, 
											     std::map<double, Mapping*>::const_iterator> > MapType;
	/** @brief Iterator type of the used InterpolateableMap type.*/
	typedef MapType::intpl_iterator IteratorType;
	
	/** @brief Iterator storing the current position inside the underlying Mappings
	 * submapping map.*/
	IteratorType valueIt;
	
	/** @brief The submapping of the submapping map at the current position.*/
	MapType::interpolated subMapping;
	
	/** @brief An iterator for the submapping which points two the current position
	 * in the next dimensions.*/
	MappingIterator* subIterator;
	
	/** @brief The MultiDimmapping to iterate over.*/
	MultiDimMapping& mapping;
	
	/** @brief The current position in every Dimension of this Iterator.*/
	Argument position;
	
	/** @brief The position a call to "next()" whould jump to.*/
	Argument nextPosition;
	
protected:
	
	/**
	 * @brief Helper method which updates the sub-iterator for the passed
	 * position. 
	 * 
	 * Called when the position of of the iterator inside the
	 * dimension this Iterator represents has changed.
	 */
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
	
	/**
	 * @brief Helper method which updates the sub-iterator and sets the position
	 * of the subiterator to its beginning.
	 * 
	 * Called when the position of of the iterator inside the
	 * dimension this Iterator represents has changed.
	 */
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
	
	/**
	 * @brief Helpermethod which updates the nextPosition member.
	 * 
	 * Called when the current position has changed.
	 */
	void updateNextPosition();
public:
	/**
	 * @brief Intializes the Iterator for the passed MultiDimMapping and sets
	 * its position two the first entry of the passed MultiDimMapping.
	 */
	MultiDimMappingIterator(MultiDimMapping& mapping);
	
	/**
	 * @brief Intializes the Iterator for the passed MultiDimMapping and sets
	 * its position two the passed position.
	 */
	MultiDimMappingIterator(MultiDimMapping& mapping, const Argument& pos);
	
	/**
	 * @brief Frees the memory allocated for the sub mappings.
	 */
	virtual ~MultiDimMappingIterator() {
		if(subIterator)
			delete subIterator;
	}
	
	/**
	 * @brief Lets the iterator point to the passed position.
	 * 
	 * The passed new position can be at arbitary places.
	 * 
	 * Has logarithmic complexity in number of dimensions and number of
	 * entries inside each dimension.
	 */
	void jumpTo(const Argument& pos);
	
	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 * 
	 * The new position has to be compared bigger than the current position
	 * 
	 * Has linear complexity over the number of entries between the current
	 * position and the passed position. This leads to nearly constant 
	 * complexity for position close together.
	 */
	void iterateTo(const Argument& pos);
	
	/**
	 * @brief Iterates to the next position of the function.
	 * 
	 * The next position depends on the implementation of the
	 * Function. 
	 * Calling this method will always work, but if their is no next
	 * entry to iterate to inside the underlying Mapping the actual
	 * position next jumps will be valid but without meaning. 
	 * Therefore "hasNext()" should be called before calling this method.
	 * 
	 * Has constant complexity.
	 */
	virtual void next();
	
	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 * 
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 * 
	 * Constant complexity.
	 */
	virtual bool inRange() const {
		return valueIt.inRange() && (subMapping.isInterpolated || (subIterator && subIterator->inRange()));
	}
	
	/**
	 * @brief Returns the current position of the iterator.
	 * 
	 * Constant complexity.
	 */
	virtual const Argument& getPosition() const {
		return position;
	}
	
	/**
	 * @brief returns the next position a call to "next()" whould jump to.
	 * 
	 * Constant complexity.
	 */
	virtual const Argument& getNextPosition() const {
		return nextPosition;
	}
	
	/**
	 * @brief Returns the value of the underlying mapping at the current
	 * position.
	 * 
	 * Has constant complexity.
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
	 * The beginning of the function depends is the position of the first
	 * entry in the underlying Mapping.
	 * 
	 * Constant complexity.
	 */
	virtual void jumpToBegin();
    
	/**
	 * @brief Returns true if the iterator has a valid next value a call to "next()"
	 * could jump to.
	 * 
	 * Constant complexity.
	 */
    virtual bool hasNext() const {
    	return valueIt.hasNext() or (subIterator and subIterator->hasNext() and valueIt.inRange());
    }
    
	/**
	 * @brief Changes the value of the function at the current 
	 * position.
	 * 
	 * Constant complexity.
	 */
	virtual void setValue(double value);
};

/**
 * @brief Implementation of the Mapping-interface which is able to represent
 * arbitary dimensional instances of Mappings by using a treelike structure
 * of submappings, each representing the values for one of the dimensions.
 * 
 * This class internally uses a map of Mappings two represent one dimension. 
 * Where every Mapping in the map represents a submapping for the values in 
 * the next dimension at that position in the this dimension. These submappings
 * can either be in turn MultiDimMappings with further submappings or they can 
 * be TimedMappings if their dimension is the time. The TimedMappings therefore
 * represent the leafes of the threelike structure.
 */
class MultiDimMapping:public Mapping {
protected:
	/** @brief The type of the used InterpolateableMap.*/
	typedef InterpolateableMap<double, Mapping*, Linear<double, Mapping*, 
											     std::map<double, Mapping*>::value_type, 
											     std::map<double, Mapping*>::const_iterator> > SubFunctionMap;
	
	/** @brief Stores the submappings for the dimension this instance represents.*/
	SubFunctionMap entries;
	
	/** @brief The dimension this instance represents.*/
	Dimension myDimension;
	
	friend class MultiDimMappingIterator;
	
protected:
	/**
	 * @brief Initializes the Mapping with the passed dimensionset as domain and
	 * the passed dimension as the dimension this instance should represent.
	 * 
	 * Also takes the interpoaltion method to use, but not yet implemented.
	 * 
	 * This constructor is only used internally to create the submappings.
	 */
	MultiDimMapping(const DimensionSet& myDims, Dimension myDim, InterpolationMethod intpl = STEPS):
			Mapping(myDims), entries(), myDimension(myDim) {} //TODO: implement interpolationmethod
	
	/**
	 * @brief Internal helper method which creates a new submapping for this
	 * MultiDimMapping instance.
	 */
	Mapping* createSubSignal() const{
		DimensionSet::const_iterator it = dimensions.find(myDimension);
		Dimension nextDim = *(--it);
		if(nextDim == Dimension::time)
			return new TimeMapping();
		else
			return new MultiDimMapping(dimensions, nextDim, LINEAR); //TODO: use interpolationmethod
	}
	
public:
	/**
	 * @brief Initializes the Mapping with the passed DimensionSet as domain.
	 * 
	 * Also takes the interpolationmethod but is not used yet.
	 */
	MultiDimMapping(const DimensionSet& myDims, InterpolationMethod intpl = STEPS):
		Mapping(myDims), entries() {
		
		//TODO: implement interpolationmethod
	
		myDimension = *(dimensions.rbegin());
	} 
	
	/**
	 * @brief Copyconstructor which assures that the submappings are deep
	 * copied instead of only their the pointers.
	 */
	MultiDimMapping(const MultiDimMapping& o);
	
	/**
	 * @brief Copy operator which assures that the submappings are deep
	 * copied instead of only their the pointers.
	 */
	const MultiDimMapping& operator=(const MultiDimMapping& o);
	
	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const { return new MultiDimMapping(*this); }
	
	/**
	 * @brief Frees the memory for the sub mappings.
	 */
	virtual ~MultiDimMapping() {
    	for(SubFunctionMap::iterator it = entries.begin();
    		it != entries.end(); it++) {
    		
    		if(it->second)
    			delete it->second;
    	}
    }
	
	/**
	 * @brief Returns the value of this Mapping at position specified
	 * by the passed Argument.
	 * 
	 * Has logarithmic complexity over the number of dimensions and the number of
	 * entries per dimension.
	 */
	virtual double getValue(const Argument& pos) const {
		double argVal = pos.getArgValue(myDimension);
		
		Interpolated<Mapping*> subM = entries.getIntplValue(argVal);
		
		if(!(*subM))
			return double();
		
		return (*subM)->getValue(pos);
	}
	
	/**
	 * @brief Changes the value of the Mapping at the specified
	 * position.
	 * 
	 * Has logarithmic complexity over the number of dimensions and the number of
	 * entries per dimension.
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
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the Mapping and can change the value the iterator points to.
	 * 
	 * The caller of this method has to delete the Iterator if not needed
	 * anymore.
	 */
	virtual MappingIterator* createIterator() {
		return new MultiDimMappingIterator(*this);
	}
	
	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the Mapping and can change the value the iterator points to.
	 * 
	 * The caller of this method has to delete the Iterator if not needed
	 * anymore.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		return new MultiDimMappingIterator(*this, pos);
	}
	
	/**
	 * @brief Returns the dimension this instance represents.
	 */ 
	Dimension getDimension() { return myDimension; }
};


#endif /*FUNCTION_H_*/
