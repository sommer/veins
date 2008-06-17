#ifndef INTERPOLATION_H_
#define INTERPOLATION_H_

#include <map>
#include <algorithm>

template<class V>
class Interpolated {
protected:
	V value;
	
public:
	bool isInterpolated;	
	
public:
	Interpolated(V v, bool isIntpl = true):
		value(v), isInterpolated(isIntpl) {}
	
	V& operator*() {
		return value;
	}
	
	V* operator->() {
		return &value;
	}
	
	bool operator==(const Interpolated<V>& other) {
		return other.value == value && other.isInterpolated == isInterpolated;
	}
	
	bool operator!=(const Interpolated<V>& other) {
		return other.value != value || other.isInterpolated != isInterpolated;
	}
};

template<class Pair, class Key>
class PairLess {
public:
	bool operator()(const Pair& p, const Key& v) {
		return p.first < v;
	}
	
	bool operator()(const Key& v, const Pair& p) {
		return v < p.first;
	}
};

/**
 * @brief Given two iterators defining a range of key-value-pairs this class
 * provides interpolation of values for an arbitary key by returning the
 * value of the next smaller entry.
 * 
 * If there is no smaller entry it returns the next bigger. 
 */
template<class Key, class V, class Pair, class InputIterator>
class NextSmaller {
public:
	typedef Interpolated<V> interpolated;
protected:
	
	PairLess<Pair, Key> comp;
public:
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const Key& pos) const{
		
		InputIterator right = std::upper_bound(first, last, pos, comp);
						
		return operator()(first, last, pos, right);
	}
	
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const Key& pos, 
						 	InputIterator upperBound) const{
		if(first == last)
			return interpolated(V());
		
		if(upperBound == first)
			return interpolated(upperBound->second);	
		
		upperBound--;
		if(upperBound->first == pos)
			return interpolated(upperBound->second, false);
		
		return interpolated(upperBound->second);
	}
};

/**
 * @brief Given two iterators defining a range of key-value-pairs this class
 * provides linear interpolation of the value at an arbitary key-position. 
 */
template<class Key, class V, class Pair, class InputIterator>
class Linear {
public:
	typedef Interpolated<V> interpolated;
protected:
	
	PairLess<Pair, Key> comp;
public:
	static double linearInterpolation(const Key& t, 
									  const Key& t0, const Key& t1, 
									  const V& v0, const V& v1){
		return v0 + (v1 - v0) * (t - t0) / (t1 - t0);
	}
	
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const Key& pos) const{
		
		InputIterator right = std::upper_bound(first, last, pos, comp);
				
		return operator()(first, last, pos, right);
	}
	
	interpolated operator()(const InputIterator& first,
						 	const InputIterator& last,
						 	const Key& pos, 
						 	InputIterator upperBound) const{
		if(first == last)
			return interpolated(V());
		
		if(upperBound == first)
			return interpolated(upperBound->second);	
		
		InputIterator right = upperBound;
		InputIterator left = --upperBound;
		
		if(left->first == pos)
			return interpolated(left->second, false);
		
		if(right == last)
			return interpolated(left->second);
			
		return interpolated(linearInterpolation(pos, left->first, right->first, left->second, right->second));
	}
};

/**
 * @brief Template for an interpolateable const iterator for any container
 * which maps from a key to a value. This doesn't necessarily has to be a
 * map, but also can be a sorted list of pairs.
 * 
 * The ConstInterpolateableIterator provides an iterator which as able to
 * iterate in arbitary steps over a iterateable number of pairs of "Key" and "Value".
 * To determine the Value for a Key which does not exist in within the iterateable
 * number of pairs it Interpolates between the nearby existing pairs. 
 * The actual Interpolation is determined by the passed Interpolator-template
 * parameter.
 * 
 * An example use whould be to be able to iterate over a std::map<double, double>
 * in arbitary steps (even at positions for which no Key exist inside the map) and
 * be able to return an interpolated Value.
 * 
 * NOTE: The ConstInterpolateableIterator will become invalid if the underlying
 * 		 data structure is changed!
 * 
 * Template parameters:
 * Pair 		- the type of the pair used as values in the container.
 * 				  Default is std::map<Key, V>::value_type (which is of type
 * 				  std::pair<Key, V>.
 * 				  The Pair type has to provide the two public members "first" and "second".
 * Key  		- The type of the "first" member of the Pair type
 * V			- the type of the "second" member of the Pair type
 * Iterator		- the type of the iterator of the container (should be a const iterator).
 * 				  Default is std::map<Key, V>::const_iterator
 * Interpolator - The Interpolation operator to use, this has to be a class which 
 * 				  overwrites the ()-operator with the following parameters:
 * 				  Interpolated operator()(const Iterator& first,
 * 							   			  const Iterator& last,
 * 							   			  const Key& pos)
 * 				  Interpolated operator()(const Iterator& first,
 * 							   			  const Iterator& last,
 * 							   			  const Key& pos,
 * 										  Iterator upperBound)
 * 				  See the NextSmaller template for an example of an Interpolator.
 * 				  Default is NextSmaller<Key, V, Pair, Iterator>.
 */
template<class Key, class V, 
		 class Pair = const typename std::map<Key, V>::value_type,
		 class Iterator = const typename std::map<Key, V>::const_iterator,
		 class Interpolator = NextSmaller<Key, V, Pair, Iterator> > 
class ConstInterpolateableIterator {
public:
	/** @brief typedef for the returned Interpolated value of this class.*/
	typedef Interpolated<V> interpolated;
protected:
	Iterator first;
	Iterator last;
	
	Iterator right;
	
	Key position;
	Interpolator interpolate;
	
	PairLess<Pair, Key> comp;
public:
	/**
	 * @brief Initializes the iterator with the passed Iterators
	 * as boundaries.
	 */
	ConstInterpolateableIterator(Iterator first, Iterator last):
		first(first), last(last), right(first), position(), interpolate(), comp(){
		
		jumpToBegin();
	}
	
	bool operator==(const ConstInterpolateableIterator& other) {
		return position == other.position && right == other.right;
	}
	
	/**
	 * @brief Moves the iterator to the passed position. This position
	 * can be any value of the Key-type.
	 */
	void jumpTo(const Key& pos) {
		if(pos == position)
			return;
		
		if(first != last)
			right = std::upper_bound(first, last, pos, comp);
		
		position = pos;
	}
	
	/**
	 * @brief Moves the iterator to the first element.
	 */
	void jumpToBegin() {
		right = first;
		if(right != last) {
			position = right->first;
			right++;
		} else {
			position = Key();
		}
	}
	
	/**
	 * @brief forward iterates the iterator to the passed position. This position
	 * can be any value of the Key-type.
	 * 
	 * This method assumes that the passed position is near the current position
	 * of the iterator. If this is the case this method will be faster than the
	 * jumpTo-method.
	 */
	void iterateTo(const Key& pos) {
		if(pos == position)
			return;
		
		while(right != last && !(pos < right->first))
			right++;
		
		position = pos;
	}
	
	/**
	 * @brief Iterates to the next entry in the underlying data structure.
	 * 
	 * If the current position is before the position of the first element of the data
	 * structure this method will iterate to the first entry.
	 * If the current position is after the position of the last element of the data
	 * structure this method will increase the current position with the ++ operator.
	 */
	void next() {
		if(hasNext()) {
			position = right->first;
			right++;
		} else
			position++;
	}
	
	Key getNextPosition(){
		if(hasNext())
			return right->first;
		else
			return position + 1;
	}
	
	/**
	 * @brief Returns true if the current position of the iterator is between the
	 * position of the first and the last entry of the data structure.
	 */
	bool inRange() const{
		if(first == last)
			return false;
		
		Iterator tail = last;
		
		return !(position < first->first) && !((--tail)->first < position);
	}
	
	/**
	 * @brief Returns true if the a call of "next()" whould increase to the position
	 * of an a valid entry of the data structure. This means if the current position
	 * is smaller than position of the last entry.
	 */
	bool hasNext() const{
		return right != last;
	}
	
	/**
	 * @brief Returns the interpolated value at the current position of the 
	 * Iterator.
	 * 
	 * See definition of Interpolated on details on the return type.
	 */
	interpolated getValue() const{
		
		return interpolate(first, last, position, right);
	}
	
	interpolated getNextValue() const{
		if(right == last)
			return interpolate(first, last, position + 1, right);
		else{
			Iterator tmp = right;
			return interpolate(first, last, right->first, ++tmp);
		}
	}
	
	/**
	 * @brief Returns the current position of the iterator.
	 */
	Key getPosition() const{
		return position;
	}
	
};

/**
 * @brief Provides an interpolateable iterator for any Container which maps
 * from keys to values which is able to change the underlying Container.
 * 
 * The underlying Container has to provide the following things:
 * - a Member "value_type" which defines the type of the Key-Value-pairs
 * - a Member "iterator" which defines the type of the iterator
 * - an "insert"-method with the following Syntax:
 * 		Iterator insert(Iterator pos, Container::value_type newEntry)
 *   which returns an iterator pointing to the newly inserted element
 * 
 * See ConstInterpolateableIterator for more details.
 */
template<class Key, class V, 
		 class Container = std::map<Key, V>,
		 class Interpolator = NextSmaller<Key, V, typename Container::value_type, typename Container::const_iterator> >		 
class InterpolateableIterator:public ConstInterpolateableIterator<Key, V, 
																		typename Container::value_type,
																		typename Container::iterator,
																		Interpolator> 
{
protected:
	typedef typename Container::value_type value_type;
	typedef typename Container::iterator Iterator;
	typedef ConstInterpolateableIterator<Key,V,
											value_type,
											Iterator,
											Interpolator> BaseClassType;
	
	Container& cont;
	
public:
	InterpolateableIterator(Container& cont):
		BaseClassType(cont.begin(), cont.end()), cont(cont) {}
	
	/**
	 * @brief: Changes (and adds if necessary) the value for the entry at the
	 * current position of the iterator to the passed value
	 */
	void setValue(const V& value) {
		//container is empty or position is smaller first entry
		if(this->right == this->first) {
			//insert new entry before first entry and store new entry as new first
			this->first = cont.insert(this->first, value_type(this->position, value));
			
		} else {
			Iterator left = this->right;
			left--;
			if(left->first == this->position) {
				left->second = value;
			} else {
				cont.insert(this->right, value_type(this->position, value));
			}
		}
	}																		
};

template<class Key, class V, 
		 class Interpolator = NextSmaller<Key, V, typename std::map<Key, V>::value_type, typename std::map<Key, V>::const_iterator> >
class InterpolateableMap:public std::map<Key, V> {
public:
	typedef std::map<Key, V> map_type;
	typedef typename map_type::const_iterator const_iterator;
	typedef typename map_type::iterator iterator;
	typedef typename map_type::value_type value_type;
	typedef InterpolateableIterator<Key, V, map_type, Interpolator> intpl_iterator;
	typedef ConstInterpolateableIterator<Key, V, const map_type, 
											typename map_type::const_iterator, 
											Interpolator> const_intpl_iterator;
	typedef Interpolated<V> interpolated;
	
protected:	
	
	Interpolator interpolate;
public:
	
	interpolated getIntplValue(const Key& pos) const {
		if(this->empty())
			return interpolated(V(), true);
		
		const_iterator it = this->lower_bound(pos);
		
		if(it != this->end() && it->first == pos)
			return interpolated(it->second, false);
		
		
		return interpolate(this->begin(), this->end(), pos, it);
	}
	
	const_intpl_iterator findIntpl(const Key& pos) const{
		const_intpl_iterator it(this->begin(), this->end());
		
		it.jumpTo(pos);
		
		return it;
	}
	
	const_intpl_iterator beginIntpl() const{
		const_intpl_iterator it(this->begin(), this->end);
		
		return it;
	}
	
	intpl_iterator findIntpl(const Key& pos) {
		intpl_iterator it(*this);
		
		it.jumpTo(pos);
		
		return it;
	}
	
	intpl_iterator beginIntpl() {
		intpl_iterator it(*this);
		
		return it;
	}
};
#endif /*INTERPOLATION_H_*/
