/*
 * SignalInterfaces.h
 *
 *  Created on: 26.08.2008
 *      Author: karl wessel
 */

#ifndef SIGNALINTERFACES_H_
#define SIGNALINTERFACES_H_

#include "MappingBase.h"

class FilledUpMapping;

/**
 * @brief This iterator takes another ConstMappingIterator and does
 * just pipe every method to the passed ConstMappingIterator.
 *
 * This class is meant to be used as base class for Iterators which
 * want to change just several parts without having to implement and pipe every
 * other method of the ConstMappingIteratorInterface.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
template<class Base>
class BaseFilteredIterator : public Base{
protected:
	Base* origIterator;

public:
	BaseFilteredIterator(Base* orig):
		origIterator(orig) {}

	virtual ~BaseFilteredIterator() {}

	virtual const Argument& getNextPosition() const { return origIterator->getNextPosition(); }

	virtual void jumpTo(const Argument& pos) { origIterator->jumpTo(pos); }

	virtual void jumpToBegin() { origIterator->jumpToBegin(); }

	virtual void iterateTo(const Argument& pos) { origIterator->iterateTo(pos); }

	virtual void next() { origIterator->next(); }

	virtual bool inRange() const { return origIterator->inRange(); }

	virtual bool hasNext() const { return origIterator->hasNext(); }

	virtual const Argument& getPosition() const { return origIterator->getPosition(); }

	virtual double getValue() const { return origIterator->getValue(); }
};

/**
 * @brief Const version of the BaseFilteredIterator. Meant to be used for
 * ConstMappingIterator instances.
 *
 * @sa BaseFilteredIterator *
 * @author Karl Wessel
 * @ingroup mappingDetail
 * */
typedef BaseFilteredIterator<ConstMappingIterator> FilteredConstMappingIterator;

/**
 * @brief Non-Const version of the BaseFilteredIterator. Meant to be used for
 * MappingIterator instances.
 *
 * @sa BaseFilteredIterator
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class FilteredMappingIterator : public BaseFilteredIterator<MappingIterator> {
public:
	FilteredMappingIterator(MappingIterator* orig):
		BaseFilteredIterator<MappingIterator>(orig) {}

	virtual ~FilteredMappingIterator() {}

	virtual void setValue(double value) { origIterator->setValue(value); }
};


/**
 * @brief Provides an implementation of the MappingIterator-
 * Interface which is able to iterate over TimeMappings.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template <class Key, class Value, class Pair, class Iterator> class Interpolator>
class TimeMappingIterator:public MappingIterator {
protected:
	/** @brief The std::map the InterpolateableMap is based on.*/
	typedef std::map<simtime_t, double> MapType;

	/** @brief The templated InterpolateableMap the underlying Mapping uses.*/
	typedef typename InterpolateableMap<simtime_t, double,
							   Interpolator<simtime_t, double,
											MapType::value_type,
											MapType::const_iterator> >::intpl_iterator IteratorType;

	/** @brief Stores the current position iterator inside the Mapping.*/
	IteratorType valueIt;

	/** @brief Stores the current position of the iterator.*/
	Argument position;

	/** @brief Stores the next position a call of "next()" would jump to.*/
	Argument nextPosition;

	bool isStepMapping;

	bool atPreStep;
protected:
	void updateNextPos(){
		simtime_t t = valueIt.getNextPosition();
		if(isStepMapping && !atPreStep){
			t.setRaw(t.raw() - 1);
		}
		nextPosition.setTime(t);
	}
public:

	/**
	 * @brief Initializes the Iterator to use the passed InterpolateableMapIterator.
	 */
	TimeMappingIterator(IteratorType it, bool isStepMapping):
		valueIt(it), isStepMapping(isStepMapping), atPreStep(false) {

		position.setTime(valueIt.getPosition());
		updateNextPos();
	}

	/**
	 * @brief Lets the iterator point to the passed position.
	 *
	 * The passed new position can be at arbitrary places.
	 *
	 * This method has logarithmic complexity.
	 */
	void jumpTo(const Argument& pos) {
		atPreStep = false;
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
		atPreStep = false;
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
		if(isStepMapping && !atPreStep){
			valueIt.iterateTo(nextPosition.getTime());
			atPreStep = true;
		} else {
			valueIt.next();
			atPreStep = false;
		}
		position.setTime(valueIt.getPosition());
		updateNextPos();
	}

	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 *
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 *
	 * THis method has constant complexity.
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
	 * @brief Returns the next position a call to "next()" would jump to.
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
 * simtime_t to double between which values can be interpolated to represent
 * a Mapping with only time as domain.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template <class Key, class Value, class Pair, class Iterator> class Interpolator>
class TimeMapping:public Mapping {
protected:
	/** @brief The type of the std::map used by the InterpolateableMap.*/
	typedef std::map<simtime_t, double> MapType;

	/** @brief Defines the used InterpolateableMap and its template-parameters.*/
	typedef InterpolateableMap<simtime_t, double,
							   Interpolator<simtime_t, double,
											MapType::value_type,
											MapType::const_iterator> > ValueMap;

	/** @brief Stores the key-entries defining the function.*/
	ValueMap entries;

	/**
	 * @brief Stores if this mapping represents a step function.
	 *
	 * Assures that the steps are considered when iterating the mapping
	 * by adding a second key-entry as short as possible before every
	 * key entry set by the user. The additional key-entry defines the
	 * value the mapping has just before the key entry the user added.
	 */
	bool isStepMapping;
public:

	/**
	 * @brief Initializes the Mapping with the passed Interpolation method.
	 */
	TimeMapping(InterpolationMethod intpl = LINEAR):
		Mapping(), entries(), isStepMapping(intpl == STEPS) {}

	/**
	 * @brief Initializes the Mapping with the passed Interpolation method.
	 */
	TimeMapping(double outOfRangeVal, InterpolationMethod intpl = LINEAR):
		Mapping(), entries(outOfRangeVal), isStepMapping(intpl == STEPS) {}

	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const { return new TimeMapping<Interpolator>(*this); }

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
		return new TimeMappingIterator<Interpolator>(entries.beginIntpl(), isStepMapping);
	}

	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function and can change the value the iterator points to.
	 *
	 * Note: The caller of this method has to delete the returned Iterator
	 * pointer if it isn't used anymore.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		return new TimeMappingIterator<Interpolator>(entries.findIntpl(pos.getTime()), isStepMapping);
	}
};


/**
 * @brief Helper-class for the MultiDimMapping which provides an Iterator
 * which linear interpolates between two other Mapping iterators. Or in
 * other words, it provides an Iterator for an linear interpolated Mapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class LinearIntplMappingIterator:public MappingIterator {
protected:
	/** @brief Iterator for the left Mapping to interpolate.*/
	ConstMappingIterator* leftIt;
	/** @brief Iterator for the right Mapping to interpolate.*/
	ConstMappingIterator* rightIt;

	/** @brief The factor defining how strong the left and the right Mapping
	 * affect the interpolation.*/
	double factor;

public:
	/**
	 * @brief Initializes the Iterator with the passed Iterators of the mappings to
	 * Interpolate and the their interpolation-factor.
	 */
	LinearIntplMappingIterator(ConstMappingIterator* leftIt, ConstMappingIterator* rightIt, double f);

	/**
	 * @brief Deletes the left and the right mapping iterator.
	 */
	virtual ~LinearIntplMappingIterator();

	/**
	 * @brief An interpolated mapping isn't really iterateable over specific
	 * values, it only provides an fast way to get several values from an
	 * Interpolated mapping.
	 */
	virtual bool hasNext() const { return false; }
	/**
	 * @brief An interpolated mapping isn't really iterateable over specific
	 * values, it only provides an fast way to get several values from an
	 * Interpolated mapping.
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
 * @brief Helper class which represents a linear interpolation between
 * two other mappings.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class LinearIntplMapping:public Mapping {
protected:
	/** @brief The left mapping to interpolate.*/
	ConstMapping* left;
	/** @brief The right mapping to interpolate*/
	ConstMapping* right;

	/** @brief The interpolation factor determining the linear interpolation
	 * between left and right mapping.*/
	double factor;

public:

	/**
	 * @brief Initializes the LinearIntplMapping with the passed left and right
	 * Mapping to interpolate by the passed interpolation value.
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
	 * The value is calculated by the following formula:
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
 * @brief Helper class (-specialization) for multiDimMapping which is used by an
 * InterpolateableMap as return value of the "getValue()" - method.
 *
 * Provides either an pointer to an actual SubMapping of the MultiDimMapping or
 * a Pointer to an temporary InterpolatedMapping between two Sub-mappings of the
 * MultiDimMapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
template<>
class Interpolated<Mapping*> {
protected:
	/** @brief Holds the temporary InterpolatedMapping if necessary.*/
	LinearIntplMapping mapping;

	/** @brief A pointer to the Mapping this class represents.*/
	Mapping* value;

	/** @brief Stores if we use the temporary IntplMapping or a external pointer.*/
	bool isPointer;
public:
	/** @brief Stores if the underlying Mapping is interpolated or not.*/
	bool isInterpolated;

public:
	/**
	 * @brief Initializes this Interpolated instance to represent the passed
	 * Interpolated Mapping. Copies the passed Mapping to its internal member.
	 * Sets "isInterpolated" to true.
	 */
	Interpolated(const LinearIntplMapping& m):
		mapping(m), isPointer(false), isInterpolated(true) {

		value = &mapping;
	}

	/**
	 * @brief Initializes this Interpolated instance to represent the Mapping
	 * the passed pointer points to and with the passed isIntpl value.
	 *
	 * The passed pointer has to be valid as long as this instance exists.
	 */
	Interpolated(Mapping* m, bool isIntpl = true):
		mapping(), value(m), isPointer(true), isInterpolated(isIntpl) {}

	/**
	 * @brief Copy-constructor which assures that the internal storage is used correctly.
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
	 * @brief Dereferences this Interpolated to the represented value (works like
	 * dereferencing an std::iterator).
	 */
	Mapping*& operator*() {
		return value;
	}

	/**
	 * @brief Dereferences this Interpolated to the represented value (works like
	 * dereferencing an std::iterator).
	 */
	Mapping** operator->() {
		return &value;
	}

	/**
	 * @brief Two Interpolated<Mapping*> are compared equal if the pointer to
	 * the represented Mapping is the same as well as the "isInterpolated"
	 * value.
	 */
	bool operator==(const Interpolated<Mapping*>& other) {
		return value == other.value && isInterpolated == other.isInterpolated;
	}

	/**
	 * @brief Two Interpolated<Mapping*> are compared non equal if the pointer to
	 * the represented Mapping differs or the "isInterpolated"
	 * value.
	 */
	bool operator!=(const Interpolated<Mapping*>& other) {
		return value != other.value || isInterpolated != other.isInterpolated;
	}
};

/**
 * @brief Specialization of the Linear-template which provides LinearInterpolation
 * for pointer two Mappings. Used by MultiDimMapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
template<>
class Linear<double, Mapping*, std::map<double, Mapping*>::value_type, std::map<double, Mapping*>::const_iterator> {
public:
	/** @brief Iterator type of the mapping over which to interpolate.*/
	typedef std::map<double, Mapping*>::const_iterator InputIterator;

	/** @brief Interpolated type used as return value.*/
	typedef Interpolated<Mapping*> interpolated;
protected:

	/** @brief Comparison class for the values of the map to interpolate over.*/
	PairLess<std::map<double, Mapping*>::value_type, double> comp;


	bool continueOutOfRange;
	Mapping* outOfRangeVal;

public:
	Linear():
		continueOutOfRange(true) {}

	Linear(Mapping* oorv):
		continueOutOfRange(false), outOfRangeVal(oorv) {}

	void setOutOfRangeVal(Mapping* oorv) { outOfRangeVal = oorv; }

	/**
	 * @brief calculates the linear interpolation factor used for the created
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
	 * either an actual entry of the interpolated map (if the position two
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
	 * either an actual entry of the interpolated map (if the position two
	 * interpolate was exactly that. Or it can be an interpolated value, if the
	 * passed position was between two entries of the map.
	 * This state can be retrieved with the "isInterpolated"-Member of the returned
	 * "interpolated".
	 */
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const double& pos,
						 	InputIterator upperBound) const{
		if(first == last){
			if(continueOutOfRange)
				return interpolated(0);
			else
				return interpolated(outOfRangeVal);
		}

		if(upperBound == first){
			if(continueOutOfRange)
				return interpolated(upperBound->second);
			else
				return interpolated(outOfRangeVal);
		}

		InputIterator right = upperBound;
		InputIterator left = --upperBound;

		if(left->first == pos)
			return interpolated(left->second, false);

		if(right == last){
			if(continueOutOfRange)
				return interpolated(left->second);
			else
				return interpolated(outOfRangeVal);
		}

		double fact = linearInterpolationFactor(pos, left->first, right->first);

		return interpolated(LinearIntplMapping(left->second, right->second, fact));
	}
};

/**
 * @brief Represents a constant mathematical mapping (f(x) = c)
 *
 * Returns the same value for every point in any dimension.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class ConstantSimpleConstMapping : public SimpleConstMapping {
protected:
	double value;

public:
	ConstantSimpleConstMapping(const DimensionSet& dims, double val):
		SimpleConstMapping(dims), value(val) {}

	ConstantSimpleConstMapping(const DimensionSet& dims,
							   const Argument& key,
							   double val):
		SimpleConstMapping(dims, key), value(val) {}

	virtual double getValue(const Argument& pos) const {
		return value;
	}

	/**
	 * @brief Returns the value of this constant mapping.
	 */
	double getValue() const {
		return value;
	}

	/**
	 * @brief Sets the value of this constant mapping.
	 */
	void setValue(double val) { value = val; }

	ConstMapping* constClone() const  {
		return new ConstantSimpleConstMapping(dimensions, value);
	}
};

/**
 * @brief Wraps an ConstMappingIterator into a MappingIterator
 * interface.
 *
 * Assumes that "setValue()" of the MappingIterator interface will
 * never be called.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class ConstMappingIteratorWrapper : public MappingIterator {
protected:
	ConstMappingIterator* iterator;
public:
	ConstMappingIteratorWrapper(ConstMappingIterator* it):
		iterator(it) {}

	virtual ~ConstMappingIteratorWrapper() {
		if(iterator)
			delete iterator;
	}

	virtual void setValue(double value) { assert(false); }

	virtual const Argument& getNextPosition() const { return iterator->getNextPosition(); }

	virtual void jumpTo(const Argument& pos) { iterator->jumpTo(pos); }

	virtual void jumpToBegin() { iterator->jumpToBegin(); }

	virtual void iterateTo(const Argument& pos) { iterator->iterateTo(pos); }

	virtual void next() { iterator->next(); }

	virtual bool inRange() const { return iterator->inRange(); }

	virtual bool hasNext() const { return iterator->hasNext(); }

	virtual const Argument& getPosition() const { return iterator->getPosition(); }

	virtual double getValue() const { return iterator->getValue(); }
};

/**
 * @brief Wraps an ConstMapping into a Mapping interface.
 *
 * Assumes that "setValue()" of the Mapping interface will
 * never be called.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class ConstMappingWrapper : public Mapping {
protected:
	ConstMapping* mapping;

public:
	ConstMappingWrapper(ConstMapping* m):
		Mapping(m->getDimensionSet()), mapping(m) {}

	virtual void setValue(const Argument& pos, double value) { assert(false); }

	virtual MappingIterator* createIterator() {
		return new ConstMappingIteratorWrapper(mapping->createConstIterator());
	}

	virtual MappingIterator* createIterator(const Argument& pos) {
		return new ConstMappingIteratorWrapper(mapping->createConstIterator(pos));
	}

	virtual double getValue(const Argument& pos) const { return mapping->getValue(pos); }

	virtual ConstMappingIterator* createConstIterator() {
		return mapping->createConstIterator();
	}

	virtual ConstMappingIterator* createConstIterator(const Argument& pos) {
		return mapping->createConstIterator(pos);
	}

	virtual ConstMapping* constClone() const { return mapping->constClone(); }

	virtual Mapping* clone() const {
		return new ConstMappingWrapper(mapping->constClone());
	}
};

template<template <class Key, class Value,
					class Pair,
					class iterator> class Interpolator>
class MultiDimMapping;

/**
 * @brief Implementation of the MappingIterator-interface which is able
 * to iterate over every value in a MultiDimMapping.
 *
 * As the MultiDimMapping has a tree-like structure of sub-mappings to
 * represent multiple dimensions, the MultiDimIterator consist of a
 * number of sub-MultiDimIterator to represent the current position
 * inside the sub-mappings. So every sub-mapping-iterator represents
 * one dimension and the and Iterator to next Dimensions.
 * The last iterator is an TimeMappingIterator.
 *
 * Iteration works by sub-iterator-first-iteration. Which means that
 * at first the sub-iterator at the current position is iterated to its
 * end before the position inside the dimension of this iterator is increased.
 * This assures the iteration order demanded by the MappingIterator-interface.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template <class Key, class Value,
					class Pair,
					class iterator> class Interpolator>
class MultiDimMappingIterator:public MappingIterator {
protected:
	/** @brief The type of the InterpolateableMap used by the underlying Mapping.*/
	typedef InterpolateableMap<double, Mapping*,
							   Interpolator<double, Mapping*,
										    std::map<double, Mapping*>::value_type,
										    std::map<double, Mapping*>::const_iterator> > MapType;
	/** @brief Iterator type of the used InterpolateableMap type.*/
	typedef typename MapType::intpl_iterator IteratorType;

	/** @brief The MultiDimmapping to iterate over.*/
	MultiDimMapping<Interpolator>& mapping;

	/** @brief Iterator storing the current position inside the underlying Mappings
	 * sub-mapping map.*/
	IteratorType valueIt;

	/** @brief The sub-mapping of the sub-mapping map at the current position.*/
	typename MapType::interpolated subMapping;

	/** @brief An iterator for the sub-mapping which points two the current position
	 * in the next dimensions.*/
	MappingIterator* subIterator;

	/** @brief The current position in every Dimension of this Iterator.*/
	Argument position;

	/** @brief The position a call to "next()" would jump to.*/
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
		typename MapType::interpolated subM = valueIt.getValue();
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
	 * of the sub-iterator to its beginning.
	 *
	 * Called when the position of of the iterator inside the
	 * dimension this Iterator represents has changed.
	 */
	void updateSubIterator() {
		typename MapType::interpolated subM = valueIt.getValue();
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
	 * @brief Helper method which updates the nextPosition member.
	 *
	 * Called when the current position has changed.
	 */
	void updateNextPosition(){
		bool intp = subMapping.isInterpolated;

		bool noSubIt = false;
		bool hasNoNext = false;
		if(!intp){
			noSubIt = !subIterator;
			if(!noSubIt)
				hasNoNext = !subIterator->hasNext();
		}
		if(intp || noSubIt || hasNoNext){
			if(valueIt.hasNext()){
				ConstMappingIterator* tmp = (*valueIt.getNextValue())->createConstIterator();
				nextPosition.setArgValues(tmp->getPosition());
				delete tmp;
			}else{
				nextPosition = position;
			}
			nextPosition.setArgValue(mapping.myDimension, valueIt.getNextPosition());

		} else {
			nextPosition.setArgValues(subIterator->getNextPosition());
		}
	}

public:
	/**
	 * @brief Initializes the Iterator for the passed MultiDimMapping and sets
	 * its position two the first entry of the passed MultiDimMapping.
	 */
	MultiDimMappingIterator(MultiDimMapping<Interpolator>& mapping):
		mapping(mapping),
		valueIt(mapping.entries.beginIntpl()),
		subMapping(0), subIterator(0),
		position(){

		subMapping = valueIt.getValue();
		if(!subMapping.isInterpolated && *subMapping) {
			subIterator = (*subMapping)->createIterator();
			position = subIterator->getPosition();
			position.setArgValue(mapping.myDimension, valueIt.getPosition());
		} else {
			position = Argument(mapping.dimensions);
		}
		nextPosition = position;

		updateNextPosition();
	}

	/**
	 * @brief Intializes the Iterator for the passed MultiDimMapping and sets
	 * its position two the passed position.
	 */
	MultiDimMappingIterator(MultiDimMapping<Interpolator>& mapping, const Argument& pos):
		mapping(mapping),
		valueIt(mapping.entries.findIntpl(pos.getArgValue(mapping.myDimension))),
		subMapping(0), subIterator(0),
		position(){

		subMapping = valueIt.getValue();
		if(*subMapping){
			subIterator = (*subMapping)->createIterator(pos);
		}

		position = pos;
		nextPosition = position;
		updateNextPosition();

	}

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
	 * The passed new position can be at arbitrary places.
	 *
	 * Has logarithmic complexity in number of dimensions and number of
	 * entries inside each dimension.
	 */
		void jumpTo(const Argument& pos){
		double argVal = pos.getArgValue(mapping.myDimension);

		if(argVal != valueIt.getPosition() && pos.hasArgVal(mapping.myDimension)) {
			valueIt.jumpTo(argVal);
			updateSubIterator(pos);
		} else {
			if(subIterator)
				subIterator->jumpTo(pos);
		}

		position.setArgValues(pos);
		nextPosition.setArgValues(position);
		updateNextPosition();
	}

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
	void iterateTo(const Argument& pos){
		double argVal = pos.getArgValue(mapping.myDimension);

		if(argVal != valueIt.getPosition() && pos.hasArgVal(mapping.myDimension)) {
			valueIt.iterateTo(argVal);
			updateSubIterator(pos);
		} else {
			if(subIterator)
				subIterator->iterateTo(pos);
		}

		position.setArgValues(pos);
		updateNextPosition();
	}

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
	virtual void next(){
		if(!subMapping.isInterpolated && subIterator && subIterator->hasNext()) {
			subIterator->next();
		} else {
			valueIt.next();
			updateSubIterator();
		}


		if(subIterator)
			position.setArgValues(subIterator->getPosition());

		position.setArgValue(mapping.myDimension, valueIt.getPosition());

		updateNextPosition();
	}

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
	 * @brief returns the next position a call to "next()" would jump to.
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
	virtual void jumpToBegin(){
		valueIt.jumpToBegin();
		updateSubIterator();
		if(subIterator)
			position.setArgValues(subIterator->getPosition());

		position.setArgValue(mapping.myDimension, valueIt.getPosition());
		updateNextPosition();
	}

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
	virtual void setValue(double value){
		if(subMapping.isInterpolated) {
			valueIt.setValue(mapping.createSubSignal());
			updateSubIterator(position);
		}
		subIterator->setValue(value);
	}
};



/**
 * @brief Implementation of the Mapping-interface which is able to represent
 * arbitrary dimensional instances of Mappings by using a tree-like structure
 * of sub-mappings, each representing the values for one of the dimensions.
 *
 * This class internally uses a map of Mappings two represent one dimension.
 * Where every Mapping in the map represents a sub-mapping for the values in
 * the next dimension at that position in the this dimension. These sub-mappings
 * can either be in turn MultiDimMappings with further sub-mappings or they can
 * be TimedMappings if their dimension is the time. The TimedMappings therefore
 * represent the leafs of the tree-like structure.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
template<template <class Key, class Value,
					class Pair,
					class iterator> class Interpolator>
class MultiDimMapping:public Mapping {
protected:
	/** @brief The type of the used InterpolateableMap.*/
	typedef InterpolateableMap<double, Mapping*,
							   Interpolator<double, Mapping*,
										    std::map<double, Mapping*>::value_type,
										    std::map<double, Mapping*>::const_iterator> > SubFunctionMap;

	/**
	 * @brief Returned by the Interpolator if the mapping is accessed outside
	 * its range (before or after the last key entry in a dimension).
	 */
	ConstantSimpleConstMapping* outOfRangeMapping;

	/**
	 * @brief Wraps the out of range mapping which is an instance of ConstMapping
	 * inside an instance of Mapping which setValue method is asserted to never be called.
	 */
	ConstMappingWrapper* wrappedOORMapping;

	/** @brief Stores the sub-mappings for the dimension this instance represents.*/
	SubFunctionMap entries;

	/** @brief The dimension this instance represents.*/
	Dimension myDimension;

	friend class MultiDimMappingIterator<Interpolator>;

	bool isMaster;

protected:
	/**
	 * @brief Initializes the Mapping with the passed DimensionSet as domain and
	 * the passed dimension as the dimension this instance should represent.
	 *
	 * Also takes the interpolation method to use, but not yet implemented.
	 *
	 * This constructor is only used internally to create the sub-mappings.
	 */
	MultiDimMapping(const DimensionSet& myDims, Dimension myDim, InterpolationMethod intpl = STEPS):
			Mapping(myDims),
			outOfRangeMapping(0),
			wrappedOORMapping(0),
			entries(),
			myDimension(myDim),
			isMaster(false) {}

	/**
	 * @brief Initializes the Mapping with the passed DimensionSet as domain and
	 * the passed dimension as the dimension this instance should represent.
	 *
	 * Also takes the interpolation method to use, but not yet implemented.
	 *
	 * This constructor is only used internally to create the sub-mappings.
	 */
	MultiDimMapping(const DimensionSet& myDims, Dimension myDim,
					ConstantSimpleConstMapping* oorm,
					ConstMappingWrapper* wrappedoorm,
					InterpolationMethod intpl = STEPS):
			Mapping(myDims),
			outOfRangeMapping(oorm),
			wrappedOORMapping(wrappedoorm),
			entries(wrappedOORMapping),
			myDimension(myDim),
			isMaster(false) {}

	/**
	 * @brief Intern copy-constructor which assures that the sub-mappings are deep
	 * copied instead of only their pointers.
	 */
	MultiDimMapping(const MultiDimMapping<Interpolator>& o,
					ConstantSimpleConstMapping* oorm,
					ConstMappingWrapper* wrappedoorm):
		Mapping(o),
		outOfRangeMapping(oorm),
		wrappedOORMapping(wrappedoorm),
		entries(o.entries),
		myDimension(o.myDimension),
		isMaster(false){

		entries.setOutOfRangeVal(wrappedOORMapping);

		copySubMappings(o);
	}

	/**
	 * @brief Internal helper method which creates a new sub-mapping for this
	 * MultiDimMapping instance.
	 */
	Mapping* createSubSignal() const{
		DimensionSet::const_iterator it = dimensions.find(myDimension);
		Dimension nextDim = *(--it);
		if(wrappedOORMapping == 0) {
			if(nextDim == Dimension::time)
				return new TimeMapping<Interpolator>();
			else
				return new MultiDimMapping<Interpolator>(dimensions, nextDim, LINEAR);
		} else {
			if(nextDim == Dimension::time)
				return new TimeMapping<Interpolator>(outOfRangeMapping->getValue());
			else
				return new MultiDimMapping<Interpolator>(dimensions, nextDim,
										   outOfRangeMapping,
										   wrappedOORMapping, LINEAR);
		}
	}

	void copySubMappings(const MultiDimMapping& o){
		DimensionSet::const_iterator dimIt = dimensions.find(myDimension);
		Dimension nextDim = *(--dimIt);

		if(nextDim == Dimension::time)
		{
			for(typename SubFunctionMap::iterator it = entries.begin();
				it != entries.end(); it++)
			{
				it->second = new TimeMapping<Interpolator>(*(static_cast<TimeMapping<Interpolator>*>(it->second)));
			}
		} else
		{
			for(typename SubFunctionMap::iterator it = entries.begin();
				it != entries.end(); it++)
			{
				if(outOfRangeMapping == 0) {
					it->second = new MultiDimMapping<Interpolator>(*(static_cast<MultiDimMapping<Interpolator>*>(it->second)));
				} else {
					it->second = new MultiDimMapping<Interpolator>(*(static_cast<MultiDimMapping<Interpolator>*>(it->second)),
													 outOfRangeMapping,
													 wrappedOORMapping);
				}
			}
		}
	}

public:
	/**
	 * @brief Initializes the Mapping with the passed DimensionSet as domain.
	 *
	 * Also takes the interpolation-method but is not used yet.
	 */
	MultiDimMapping(const DimensionSet& myDims, InterpolationMethod intpl = STEPS):
		Mapping(myDims),
		outOfRangeMapping(0),
		wrappedOORMapping(0),
		entries(),
		isMaster(true){

		myDimension = *(dimensions.rbegin());
	}

	/**
	 * @brief Initializes the Mapping with the passed DimensionSet as domain.
	 *
	 * Also takes the interpolation-method but is not used yet.
	 */
	MultiDimMapping(const DimensionSet& myDims, double oorv, InterpolationMethod intpl = STEPS):
		Mapping(myDims),
		outOfRangeMapping(new ConstantSimpleConstMapping(myDims, oorv)),
		wrappedOORMapping(new ConstMappingWrapper(outOfRangeMapping)),
		entries(wrappedOORMapping),
		isMaster(true) {

		myDimension = *(dimensions.rbegin());
	}

	/**
	 * @brief Copy-constructor which assures that the sub-mappings are deep
	 * copied instead of only their the pointers.
	 */
	MultiDimMapping(const MultiDimMapping<Interpolator>& o):
		Mapping(o),
		outOfRangeMapping(o.outOfRangeMapping),
		wrappedOORMapping(o.wrappedOORMapping),
		entries(o.entries),
		myDimension(o.myDimension),
		isMaster(true){

		if(outOfRangeMapping != 0){
			outOfRangeMapping = new ConstantSimpleConstMapping(dimensions, o.outOfRangeMapping->getValue());
			wrappedOORMapping = new ConstMappingWrapper(outOfRangeMapping);
			entries.setOutOfRangeVal(wrappedOORMapping);
		}


		copySubMappings(o);
	}

	/**
	 * @brief Copy operator which assures that the sub-mappings are deep
	 * copied instead of only their the pointers.
	 */
	const MultiDimMapping& operator=(const MultiDimMapping<Interpolator>& o){
		for(typename SubFunctionMap::iterator it = entries.begin();
			it != entries.end(); it++) {

			if(it->second)
				delete it->second;
		}

		dimensions = o.dimensions;
		entries = o.entries;
		myDimension = o.myDimension;
		outOfRangeMapping = o.outOfRangeMapping;
		wrappedOORMapping = o.wrappedOORMapping;
		isMaster = true;

		if(outOfRangeMapping != 0){
			outOfRangeMapping = new ConstantSimpleConstMapping(dimensions, o.outOfRangeMapping->getValue());
			wrappedOORMapping = new ConstMappingWrapper(outOfRangeMapping);
			entries.setOutOfRangeVal(wrappedOORMapping);
		}

		copySubMappings(o);

		return *this;
	}

	/**
	 * @brief returns a deep copy of this mapping instance.
	 */
	virtual Mapping* clone() const { return new MultiDimMapping<Interpolator>(*this); }

	/**
	 * @brief Frees the memory for the sub mappings.
	 */
	virtual ~MultiDimMapping() {
    	for(typename SubFunctionMap::iterator it = entries.begin();
    		it != entries.end(); it++) {

    		if(it->second)
    			delete it->second;
    	}

    	if(isMaster) {
    		if(outOfRangeMapping)
    			delete outOfRangeMapping;
    		if(wrappedOORMapping)
    			delete wrappedOORMapping;
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

	    typename SubFunctionMap::iterator posIt = entries.lower_bound(argVal);

		if(posIt == entries.end() || posIt->first != argVal) {
			Mapping* subF = createSubSignal();
	    	posIt = entries.insert(posIt, typename SubFunctionMap::value_type(argVal, subF));
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
		return new MultiDimMappingIterator<Interpolator>(*this);
	}

	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the Mapping and can change the value the iterator points to.
	 *
	 * The caller of this method has to delete the Iterator if not needed
	 * anymore.
	 */
	virtual MappingIterator* createIterator(const Argument& pos) {
		return new MultiDimMappingIterator<Interpolator>(*this, pos);
	}

	/**
	 * @brief Returns the dimension this instance represents.
	 */
	Dimension getDimension() { return myDimension; }
};





/**
 * @brief MappingIterator implementation for FilledUpMappings.
 *
 * Assures that although FilledUpMapping is an Mapping instance the
 * "setValue()"-method may never be called.
 *
 * @sa FilledUpMapping
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class FilledUpMappingIterator : public MultiDimMappingIterator<Linear>{
public:
	FilledUpMappingIterator(FilledUpMapping& mapping);

	FilledUpMappingIterator(FilledUpMapping& mapping, const Argument& pos);

	virtual void setValue(double value) {
		assert(false);
	}
};

/**
 * @brief Takes a source ConstMapping with a domain A and a set of KeyEntries
 * for a domain B and creates a clone of the source mapping with the domain B
 * and the KeyEntries passed.
 *
 * This class is used by "applyElementWiseOperator()"-method to be able
 * to handle cases where the second mappings domain is a real subset of
 * the first mappings domain (meaning the first mappings domain has the same
 * dimensions as the seconds domain and at least one further dimension).
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class FilledUpMapping : public MultiDimMapping<Linear> {
//--------members----------
public:
	typedef std::set<double> KeySet;
	typedef std::map<Dimension, KeySet > KeyMap;
protected:
	Mapping* fillRef;
	const KeyMap* keys;

//--------methods----------
protected:
	void fillRefIfNecessary() {
		KeyMap::const_iterator it = keys->find(myDimension);

		if(it == keys->end())
			return;

		fillRef = createSubSignal();

		for (KeySet::const_iterator keyIt = it->second.begin();
			 keyIt != it->second.end(); ++keyIt)
		{
			entries.insert(entries.end(), SubFunctionMap::value_type(*keyIt, fillRef));
		}
	}

	FilledUpMapping(const DimensionSet& myDims, Dimension myDim, const KeyMap* keys, InterpolationMethod intpl = STEPS):
		MultiDimMapping<Linear>(myDims, myDim, intpl), fillRef(0), keys(keys)
	{
		fillRefIfNecessary();
	}

	Mapping* createSubSignal() const{
		DimensionSet::const_iterator it = dimensions.find(myDimension);
		Dimension nextDim = *(--it);
		if(nextDim == Dimension::time)
			return new TimeMapping<Linear>();
		else
			return new FilledUpMapping(dimensions, nextDim, keys, LINEAR);
	}
public:
	FilledUpMapping(ConstMapping* source, const DimensionSet& dims, const KeyMap* keys):
		MultiDimMapping<Linear>(dims), fillRef(0), keys(keys)
	{
		ConstMappingIterator* it = source->createConstIterator();

		if(it->inRange())
		{
			fillRefIfNecessary();

			while(it->inRange()){
				appendValue(it->getPosition(), it->getValue());

				if(!it->hasNext())
					break;

				it->next();
			}
		}

		delete it;

		keys = 0;
	}

	virtual ~FilledUpMapping(){
		if(fillRef != 0){
			delete fillRef;
			entries.clear();
		}
	}

	virtual void appendValue(const Argument& pos, double value) {
		assert(keys != 0);

		if(fillRef != 0)
		{
			fillRef->appendValue(pos, value);
		}
		else
		{
			double argVal = pos.getArgValue(myDimension);

			SubFunctionMap::iterator posIt = entries.lower_bound(argVal);

			if(posIt == entries.end() || posIt->first != argVal) {
				Mapping* subF = createSubSignal();
				posIt = entries.insert(posIt, SubFunctionMap::value_type(argVal, subF));
			}

			posIt->second->appendValue(pos, value);
		}

	}

	virtual MappingIterator* createIterator() {
		return new FilledUpMappingIterator(*this);
	}

	virtual MappingIterator* createIterator(const Argument& pos) {
		return new FilledUpMappingIterator(*this, pos);
	}
};


//TODO: implement mapping operation which have a defined boundary.
/**
 * @brief TODO: auto generated doc
 */
/*
class BoundedConstMappingIterator : public FilteredConstMappingIterator{
//--------members----------
protected:
	simtime_t boundBegin;
	simtime_t boundEnd;

//--------methods----------
protected:
	void checkAndUpdatePosition(){
		Argument pos = origIterator->getPosition();

		if(pos.getTime() < boundBegin){
			pos.setTime(boundBegin);
			origIterator->jumpTo(pos);
		} else if(pos.getTime() > boundEnd) {

		}
	}

public:
	BoundedConstMappingIterator(ConstMapping* mapping,
								const simtime_t& boundStart,
								const simtime_t& boundEnd):
		FilteredConstMappingIterator(mapping->createConstIterator()),
		boundBegin(boundStart),
		boundEnd(boundEnd)
	{
		assert(boundStart <= boundEnd);


	}



};
*/

/**
 * @brief Provides several utility methods for Mappings.
 *
 * @author Karl Wessel
 * @ingroup mapping
 */
class MappingUtils {
private:
	static ConstMapping* mappingBuffer;

private:
	static void freeMappingBuffer(){
		if(mappingBuffer != 0){
			delete mappingBuffer;
			mappingBuffer = 0;
		}
	}

	static void setMappingBuffer(ConstMapping* mapping){
		assert(mappingBuffer == 0);

		mappingBuffer = mapping;
	}

	static ConstMapping* createCompatibleMapping(ConstMapping& src, ConstMapping& dst);

	static bool iterateToNext(ConstMappingIterator* it1, ConstMappingIterator* it2);

public:

	/**
	 * @brief Returns an appropriate changeable Mapping with the specified domain
	 * and the specified interpolation method.
	 *
	 * Note: The interpolation method is always linear, at the moment.
	 */
	static Mapping* createMapping(const DimensionSet& domain = DimensionSet(Dimension::time),
								  Mapping::InterpolationMethod intpl = Mapping::LINEAR);

	/**
	 * @brief Returns an appropriate changeable Mapping with the specified domain
	 * and the specified interpolation method.
	 *
	 * Note: The interpolation method is always linear, at the moment.
	 */
	static Mapping* createMapping(double outOfRangeValue,
								  const DimensionSet& domain = DimensionSet(Dimension::time),
								  Mapping::InterpolationMethod intpl = Mapping::LINEAR);

	template<class Operator>
	static Mapping* applyElementWiseOperator(ConstMapping& f1, ConstMapping& f2,
											 const Argument& intvlStart,
											 const Argument& intvlEnd,
											 Operator op){

		return 0;
	}

	template<class Operator>
	static Mapping* applyElementWiseOperator(ConstMapping& f1, ConstMapping& f2, Operator op,
											 double outOfRangeVal = 0.0,
											 bool contOutOfRange = true){
		const DimensionSet& domain1 = f1.getDimensionSet();

		ConstMapping* f2Comp = createCompatibleMapping(f2, f1);

		Mapping* result = 0;
		if(contOutOfRange)
			result = MappingUtils::createMapping(domain1);
		else
			result = MappingUtils::createMapping(outOfRangeVal, domain1);

		ConstMappingIterator* itF1 = f1.createConstIterator();
		ConstMappingIterator* itF2 = f2Comp->createConstIterator();

		if(!itF1->inRange() && !itF2->inRange()){
			delete itF1;
			delete itF2;
			return result;
		}

		MappingIterator* itRes = 0;

		if(itF1->inRange() && (!itF2->inRange() || itF1->getPosition() < itF2->getPosition())){
			itF2->jumpTo(itF1->getPosition());
		} else {
			itF1->jumpTo(itF2->getPosition());
		}

		itRes = result->createIterator(itF1->getPosition());

		while(itF1->inRange() || itF2->inRange()) {
			assert(itF1->getPosition().isSamePosition(itF2->getPosition()));

			double prod = op(itF1->getValue(), itF2->getValue());
			//result->setValue(itF1->getPosition(), prod);
			itRes->setValue(prod);

			if(!iterateToNext(itF1, itF2))
				break;

			itRes->iterateTo(itF1->getPosition());
		}

		delete itF1;
		delete itF2;
		delete itRes;

		freeMappingBuffer();

		return result;
	}



	/**
	 * @brief Multiplies the passed functions element-wise with each other
	 * and returns the result in a new Function.
	 *
	 * The domain (DimensionSet) of the result is defined by the domain
	 * of the first operand.
	 * The domain of the second Mapping has to be a subset of the domain of
	 * the first mapping.
	 */
	static Mapping* multiply(ConstMapping& f1, ConstMapping& f2);
	static Mapping* add(ConstMapping& f1, ConstMapping& f2);
	static Mapping* subtract(ConstMapping& f1, ConstMapping& f2);
	static Mapping* divide(ConstMapping& f1, ConstMapping& f2);

	static Mapping* multiply(ConstMapping& f1, ConstMapping& f2, double outOfRangeVal);
	static Mapping* add(ConstMapping& f1, ConstMapping& f2, double outOfRangeVal);
	static Mapping* subtract(ConstMapping& f1, ConstMapping& f2, double outOfRangeVal);
	static Mapping* divide(ConstMapping& f1, ConstMapping& f2, double outOfRangeVal);

	/**
	 * @brief Iterates over the passed mapping and returns value at the key entry
	 * with the highest value.
	 */
	static double findMax(ConstMapping& m);

	/**
	 * @brief Iterates over the passed mapping and returns the value at the key
	 * entry with the highest value in the range defined by the passed min and
	 * max parameter.
	 *
	 * The area defined by the min and max parameter is the number of key entries
	 * which position in each dimension is bigger or equal than the value of the min
	 * parameter in that dimension and smaller or equal than max parameter in
	 * that dimension.
	 *
	 * NOTE: This method currently does only work for one dimensional (time) mappings!
	 *
	 * TODO: implement for multidimensional mappings
	 */
	static double findMax(ConstMapping& m, const Argument& min, const Argument& max);

	/**
	 * @brief Iterates over the passed mapping and returns value at the key entry
	 * with the smallest value.
	 */
	static double findMin(ConstMapping& m);

	/**
	 * @brief Iterates over the passed mapping and returns the value at the key
	 * entry with the smallest value in the range defined by the passed min and
	 * max parameter.
	 *
	 * The area defined by the min and max parameter is the number of key entries
	 * which position in each dimension is bigger or equal than the value of the min
	 * parameter in that dimension and smaller or equal than max parameter in
	 * that dimension.
	 *
	 * NOTE: This method currently does only work for one dimensional (time) mappings!
	 *
	 * TODO: implement for multidimensional mappings
	 */
	static double findMin(ConstMapping& m, const Argument& min, const Argument& max);


	/*
	static Mapping* multiply(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
	static Mapping* add(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
	static Mapping* subtract(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
	static Mapping* divide(ConstMapping& f1, ConstMapping& f2, const Argument& from, const Argument& to);
	*/
};



/**
 * @brief Deletes its ConstMapping when this iterator is deleted.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
class ConcatConstMappingIterator : public FilteredConstMappingIterator{
//--------members----------
protected:
	ConstMapping* baseMapping;

public:
	ConcatConstMappingIterator(ConstMapping* baseMapping):
		FilteredConstMappingIterator(baseMapping->createConstIterator()),
		baseMapping(baseMapping) {}

	ConcatConstMappingIterator(ConstMapping* baseMapping, const Argument& pos):
		FilteredConstMappingIterator(baseMapping->createConstIterator(pos)),
		baseMapping(baseMapping) {}

	virtual ~ConcatConstMappingIterator() {
		delete origIterator;
		delete baseMapping;
	}
};

/**
 * @brief Defines it values by concatenating one or more
 * Mappings to a reference Mapping.
 *
 * @author Karl Wessel
 * @ingroup mappingDetail
 */
template<class Operator>
class ConcatConstMapping: public ConstMapping {
protected:
	typedef std::pair<Dimension, Argument::const_iterator> DimIteratorPair;
	typedef std::list<ConstMapping*> MappingSet;
	MappingSet mappings;
	ConstMapping* refMapping;
	Operator op;

public:
	/**
	 * @brief Initializes with the passed reference Mapping, the operator
	 * and the Mappings defined by the passed iterator.
	 */
	template<class Iterator>
	ConcatConstMapping(ConstMapping* refMapping,
					   Iterator first, Iterator last,
					   Operator op = Operator()):
		ConstMapping(refMapping->getDimensionSet()),
		refMapping(refMapping), op(op)
	{
		while(first != last){
			mappings.push_back(*first);
			++first;
		}
	}

	/**
	 * @brief Initializes with the passed reference Mapping, the operator
	 * and another Mapping to concatenate.
	 */
	ConcatConstMapping(ConstMapping* refMapping, ConstMapping* other,
					   Operator op = Operator()):
		ConstMapping(refMapping->getDimensionSet()),
		refMapping(refMapping), op(op)
	{
		mappings.push_back(other);
	}

	/**
	 * @brief Adds another Mapping to the list of Mappings to
	 * concatenate.
	 */
	void addMapping(ConstMapping* m) {
		mappings.push_back(m);
	}

	virtual double getValue(const Argument& pos) const {
		MappingSet::const_iterator it = mappings.begin();
		double res = refMapping->getValue(pos);

		for (MappingSet::const_iterator it = mappings.begin();
			 it != mappings.end(); ++it)
		{
			res = op(res, (*it)->getValue(pos));
		}

		return res;
	}

	/**
	 * @brief Returns the concatenated Mapping.
	 */
	Mapping* createConcatenatedMapping(){
		MappingSet::const_iterator it = mappings.begin();

		Mapping* result = MappingUtils::applyElementWiseOperator(*refMapping, **it, op);

		while(++it != mappings.end()){
			Mapping* buf = result;
			result = MappingUtils::applyElementWiseOperator(*buf, **it, op);
			delete buf;
		}

		return result;
	}

	virtual ConstMappingIterator* createConstIterator() {
		return new ConcatConstMappingIterator(createConcatenatedMapping());
	}

	virtual ConstMappingIterator* createConstIterator(const Argument& pos) {
		return new ConcatConstMappingIterator(createConcatenatedMapping(), pos);
	}

	virtual ConstMapping* constClone() const {
		return new ConcatConstMapping(*this);
	}

	/**
	 * @brief Returns the pointer to the reference mapping.
	 */
	ConstMapping* getRefMapping() {
		return refMapping;
	}
};

/**
 * @brief Iterator for a DelayedMapping.
 *
 * @sa DelayedMapping
 * @ingroup mappingDetail
 * @author Karl Wessel
 */
class DelayedMappingIterator: public FilteredMappingIterator {
protected:
	simtime_t delay;

	Argument position;
	Argument nextPosition;
protected:
	Argument undelayPosition(const Argument& pos) const{
		Argument res(pos);
		res.setTime(res.getTime() - delay);
		return res;
	}

	Argument delayPosition(const Argument& pos) const {
		Argument res(pos);
			res.setTime(res.getTime() + delay);
			return res;
	}

	void updatePosition() {
		nextPosition = delayPosition(origIterator->getNextPosition());
		position = delayPosition(origIterator->getPosition());
	}

public:
	DelayedMappingIterator(MappingIterator* it, simtime_t delay):
		FilteredMappingIterator(it), delay(delay) {

		updatePosition();
	}

	virtual const Argument& getNextPosition() const { return nextPosition; }

	virtual void jumpTo(const Argument& pos) {
		origIterator->jumpTo(undelayPosition(pos));
		updatePosition();
	}

	virtual void jumpToBegin() {
		origIterator->jumpToBegin();
		updatePosition();
	}

	virtual void iterateTo(const Argument& pos) {
		origIterator->iterateTo(undelayPosition(pos));
		updatePosition();
	}

	virtual void next() {
		origIterator->next();
		updatePosition();
	}

	virtual const Argument& getPosition() const {
		return position;
	}
};

/**
 * @brief Moves another Mapping in its time dimension.
 *
 * See propgation delay effect of the signal for an example
 * how to use this mapping.
 *
 * @ingroup mappingDetail
 * @author Karl Wessel
 */
class DelayedMapping: public Mapping {
protected:
	Mapping* mapping;
	simtime_t delay;


protected:
	Argument delayPosition(const Argument& pos) const {
		Argument res(pos);
		res.setTime(res.getTime() - delay);
		return res;
	}

public:
	DelayedMapping(Mapping* mapping, simtime_t delay):
		Mapping(mapping->getDimensionSet()), mapping(mapping), delay(delay) {}

	virtual ~DelayedMapping() {}

	virtual double getValue(const Argument& pos) const {
		return mapping->getValue(delayPosition(pos));
	}

	virtual void setValue(const Argument& pos, double value) {
		mapping->setValue(delayPosition(pos), value);
	}

	virtual Mapping* clone() const {
		return new DelayedMapping(mapping->clone(), delay);
	}

	virtual MappingIterator* createIterator() {
		return new DelayedMappingIterator(mapping->createIterator(), delay);
	}

	virtual MappingIterator* createIterator(const Argument& pos) {
		return new DelayedMappingIterator(mapping->createIterator(delayPosition(pos)), delay);
	}

	/**
	 * @brief Returns the delay used by this mapping.
	 */
	virtual simtime_t getDelay() const {
		return delay;
	}

	/**
	 * @brief Changes the delay to the passed value.
	 */
	virtual void delayMapping(simtime_t d) {
		delay = d;
	}
};


Mapping* operator*(ConstMapping& f1, ConstMapping& f2);
Mapping* operator/(ConstMapping& f1, ConstMapping& f2);
Mapping* operator+(ConstMapping& f1, ConstMapping& f2);
Mapping* operator-(ConstMapping& f1, ConstMapping& f2);

#endif /* SIGNALINTERFACES_H_ */
