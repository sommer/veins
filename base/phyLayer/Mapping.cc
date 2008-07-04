#include "Mapping.h"
#include <assert.h>
//---Dimension implementation-----------------------------

int Dimension::nextFreeID = 1;
Dimension::DimensionIDMap Dimension::dimensionIDs;
Dimension::DimensionNameMap Dimension::dimensionNames;
const Dimension Dimension::time = Dimension("time");

int Dimension::getDimensionID(const std::string& name)
{
	DimensionIDMap::iterator it = dimensionIDs.lower_bound(name);
	
	if(it == dimensionIDs.end() || it->first != name){
		int newID = 0;
		
		//time gets its own id to make sure it has the smallest
		if(name == "time")
			newID = 0;
		else
			newID = nextFreeID++;
		
		it = dimensionIDs.insert(it, DimensionIDMap::value_type(name, newID));
		dimensionNames[newID] = name;
	}
	
	return it->second;
}

Dimension::Dimension(const std::string & name):
	id(getDimensionID(name)){}

bool Dimension::operator ==(const Dimension & other) const
{
	return id == other.id;
}

bool Dimension::operator <(const Dimension & other) const
{
	return id < other.id;
}

//--Argument implementation---------------------------

Argument::Argument(simtime_t timeVal):
	time(timeVal), values(), count(0) {}

Argument::Argument(const DimensionSet & dims, simtime_t timeVal):
	time(timeVal), values(), count(0)
{
	DimensionSet::const_iterator it = dims.begin();
	
	assert((*it) == Dimension::time);
	
	it++;
	/*
	while(it != dims.end()) {		
		values[*it] = 0.0;
		it++;
	}*/
	while(it != dims.end()) {		
		values[count] = std::pair<Dimension, double>(*it, 0.0);
		count++;
		it++;
	}
}

simtime_t Argument::getTime() const
{
	return time;
}

void Argument::setTime(simtime_t time)
{
	this->time = time;
}

bool Argument::hasArgVal(const Dimension& dim) const{
	assert(!(dim == Dimension::time));
	
	int i = 0;
	while(i < count && !(dim < values[i].first)){
		if(values[i].first == dim)
			return true;
		i++;
	}
	
	return false;
}

double Argument::getArgValue(const Dimension & dim) const
{
	assert(!(dim == Dimension::time));
	/*
	ArgValMap::const_iterator it = values.find(dim);
	
	if(it == values.end())
		return double();
	
	return it->second;*/
	int i = 0;
	while(i < count && !(dim < values[i].first)){
		if(values[i].first == dim)
			return values[i].second;
		i++;
	}
	
	return double();
}

void Argument::setArgValue(const Dimension & dim, double value)
{
	assert(!(dim == Dimension::time));
	//values[dim] = value;
	
	insertValue(0, dim, value);
	/*int i = 0;
	while(i < count && !(dim < values[i].first)){
		if(values[i].first == dim){
			values[i].second = value;
			return;
		}
		i++;
	}
	if(i == count){
		values[count++] = std::pair<Dimension, double>(dim, value);
	} else {
		count++;
		std::pair<Dimension, double> n = values[i];
		values[i] = std::pair<Dimension, double>(dim, value);
		while(i++ < count){
			std::pair<Dimension, double> tmp = values[i];
			values[i] = n;
			n = tmp;
		}
	}*/
}

int Argument::insertValue(int pos, const Dimension& dim, double value, bool ignoreUnknown){
	while(pos < count && !(dim < values[pos].first)){
		
		if(values[pos].first == dim){
			values[pos].second = value;
			return pos;
		}
		++pos;
	}
	
	if(ignoreUnknown)
		return pos;
	
	if(pos == count){
		values[count++] = std::pair<Dimension, double>(dim, value);
	} else {
		count++;
		std::pair<Dimension, double> n = values[pos];
		values[pos] = std::pair<Dimension, double>(dim, value);
		while(pos++ < count){
			std::pair<Dimension, double> tmp = values[pos];
			values[pos] = n;
			n = tmp;
		}
	}
	
	return pos;
}

void Argument::setArgValues(const Argument& o, bool ingoreUnknown){
	time = o.time;
	
	int pos = 0;
	for(int i = 0; i < o.count; i++){
		pos = insertValue(pos, o.values[i].first, o.values[i].second, ingoreUnknown);
	}
}

bool Argument::isSamePosition(const Argument & o) const
{	
	if(count < o.count){
		return false;
	}
	
	//if(fabs(time - o.time) > 0.000001){
	if(time != o.time){
		return false;
	}
	
	if(o.count == 0)
		return true;
	
	unsigned int itO = 0;		
	unsigned int it = 0;		
	
	while (it < count)
	{
		if (o.values[itO].first < values[it].first) {
			break;
		} else if (values[it].first < o.values[itO].first) 
			++it;
		else { 
			//if((fabs(values[it].second - o.values[itO].second) > 0.000001)){
			if(values[it].second != o.values[itO].second){
				break;
			}
			++it; 
			++itO; 
		}
		if (itO == o.count) return true;
	}
	
	return false;
}

bool Argument::isClose(const Argument& o, double epsilon) const{
	if(count != o.count)
		return false;
	
	if(fabs(time - o.time) > epsilon)
		return false;
	
	unsigned int itO = 0;
	for(unsigned int it = 0;
		it < count; it++) {
		
		if(!(values[it].first == o.values[itO].first) || (fabs(values[it].second - o.values[itO].second) > epsilon)){
			return false;
		}
		itO++;
	}
	
	return true;
}

bool Argument::operator==(const Argument & o) const
{
	if(count != o.count)
		return false;
	
	if(time != o.time)
		return false;
	
	unsigned int itO = 0;
	for(unsigned int it = 0;
		it < count; it++) {
		
		if(!(values[it].first == o.values[itO].first) || (values[it].second != o.values[itO].second)){
			return false;
		}
		itO++;
	}
	
	return true;
}

void Argument::operator=(const Argument& o){
	count = o.count;
	
	memcpy(values, o.values, sizeof(std::pair<Dimension, double>) * count);
	//for(unsigned int i = 0; i < count; i++)
	//	values[i] = o.values[i];
		
	time = o.time;
	
}

bool Argument::operator<(const Argument & o) const
{
	assert(getDimensions() == o.getDimensions());
	
	for(int it = (int)o.count - 1; it >= 0; --it){
		double diff = values[it].second - o.values[it].second;
		//if(fabs(diff) > 0.000001){
		if(diff != 0){
			return diff < 0.0;
		}
	}
	
	return (time - o.time) < 0;
}

double Argument::compare(const Argument& o, const DimensionSet& dims) const{
	DimensionSet::const_reverse_iterator rIt = dims.rbegin();
	
	int ind = (int)count - 1;
	int indO = (int)o.count - 1;
	
	//iterate through passed dimensions and compare arguments in these dimensions
	while(rIt != dims.rend()){
		const Dimension& dim = *rIt;
		
		//catch special case time (after which we can abort)
		if(dim == Dimension::time){
			double diff = time - o.time;
			//if(fabs(diff) < 0.000001)
			if(diff == 0)
				return 0;
			
			return diff;
		}
		
		//iterate indices to the passed dimensions or the next smaller
		while(ind >= 0 && dim < values[ind].first) 
			--ind;
		while(indO >= 0 && dim < o.values[indO].first)
			--indO;
		
		//if the last dimensions could not be found the arguments are assumed equal
		if(ind < 0 || indO < 0){
			return 0;
		}
		
		//if both Arguments are defined in the current dimensions
		//compare them (otherwise we asume them equal and continue)
		if(values[ind].first == dim && o.values[indO].first == dim){
			double diff = values[ind].second - o.values[indO].second;
			
			//if(fabs(diff) > 0.000001)
			if(diff != 0)
				return diff;
		}
		++rIt;
	}
	return 0;
}

/*bool Argument::operator<(const Argument & o) const
{
	assert(getDimensions().isSubSet(o.getDimensions()));
		
	if(count < o.count){
		return false;
	}
	
	int itO = (int)o.count - 1;		
	int it = (int)count - 1;		
	
	while (it >= 0 && itO >= 0)
	{
		//o has a dimensions this Argument doesn't have (o isn't subset)->invalid
		if (o.values[itO].first < values[it].first) {
			return false;
		//o doesn't have the current dimension of Argument, we assume its equal
		} else if (values[it].first < o.values[itO].first) 
			--it;
		//o and this Argument have this dimension->check for smaller
		else { 
			if((values[it].second - o.values[itO].second) < -0.00001){
				return true;
			}
			--it; 
			--itO; 
		}
		if (itO == o.count) break;
	}
	
	return time - o.time < -0.00001;
}*/

//---ConstMapping implementation----------------------------------
const Argument& min(const Argument& a1, const Argument& a2){
	if(a1 < a2)
		return a1;
	
	return a2;
}

bool iterateToNextSubSet(ConstMappingIterator* it1, ConstMappingIterator* it2, const DimensionSet& d1WithoutD2, const DimensionSet& d2){
	bool it1HasNext = it1->hasNext();
	bool it2HasNext = it2->hasNext();
	
	if(it1HasNext || it2HasNext){
		if(it1->getPosition().compare(it1->getNextPosition(), d1WithoutD2) != 0){
			if(it2HasNext){
				it2->next();
				it1->iterateTo(it2->getPosition());
			} else {
				it2->jumpToBegin();
				if(it1->getNextPosition().compare(it2->getPosition(), d2) < 0){
					it1->next();
					it2->jumpTo(it1->getPosition());
				} else {
					Argument nextPos = it1->getNextPosition();
					nextPos.setArgValues(it2->getPosition());
					it1->iterateTo(nextPos);
				}
			}
		}else{		
			if(!it2HasNext || it1->getNextPosition().compare(it2->getNextPosition(), d2) < 0){
				it1->next();
				it2->iterateTo(it1->getPosition());
							
			} else {
				it2->next();
				it1->iterateTo(it2->getPosition());
			}
		}
		return true;
	} else {
		return false;
	}
}

bool iterateToNext(ConstMappingIterator* it1, ConstMappingIterator* it2){
	bool it1HasNext = it1->hasNext();
	bool it2HasNext = it2->hasNext();
	
	if(it1HasNext || it2HasNext){
		if(!it2HasNext || it1->getNextPosition() < it2->getNextPosition()){
			it1->next();
			it2->iterateTo(it1->getPosition());
						
		} else {
			it2->next();
			it1->iterateTo(it2->getPosition());
		}
		
		return true;
	} else {
		return false;
	}
}

void set_difference(const DimensionSet& d1, const DimensionSet& d2, DimensionSet& result){
	Dimension lastD2Dim = *d2.rbegin();
	for(DimensionSet::const_reverse_iterator it = d1.rbegin();
		!(*it == lastD2Dim); ++it){
		result.insert(result.end(), *it);
	}
}

bool isSimpleSubSet(const DimensionSet& d1, const DimensionSet& d2){
	DimensionSet::const_reverse_iterator it2 = d2.rbegin();
	DimensionSet::const_reverse_iterator it = d1.rbegin();
	
	while(!(*it == *it2)){
		++it;
	}
	++it;
	++it2;
	
	while(it != d1.rend()){
		if(it2 == d2.rend())
			return false;
		
		if(!(*it == *it2))
			return false;
		++it;
		++it2;
	}
	
	return true;
}

Mapping* multiplySubSet(ConstMapping& f1, ConstMapping& f2){
	const DimensionSet& domain1 = f1.getDimensionSet();
	const DimensionSet& domain2 = f2.getDimensionSet();
	
	assert(isSimpleSubSet(domain1, domain2));
	
	DimensionSet d1WithoutD2;
	
	//std::set_difference(domain1.begin(), domain1.end(), domain2.begin(), domain2.end(), (DimensionSet::iterator)d1WithoutD2.begin());
	set_difference(domain1, domain2, d1WithoutD2);
	
	Mapping* result = Mapping::createMapping(domain1);
	
	ConstMappingIterator* itF1 = f1.createConstIterator();
	ConstMappingIterator* itF2 = f2.createConstIterator();
	MappingIterator* itRes = result->createIterator();
	
	if(itF1->getPosition().compare(itF2->getPosition(), domain2) < 0){
		itF2->jumpTo(itF1->getPosition());		
	} else {
		itF1->jumpTo(itF2->getPosition());
	}
	itRes->jumpTo(itF1->getPosition());	
	
	while(itF1->inRange() || itF2->inRange()) {
		assert(itF1->getPosition().isSamePosition(itF2->getPosition()));
		
		double prod = itF1->getValue() * itF2->getValue();
		//result->setValue(itF1->getPosition(), prod);
		itRes->setValue(prod);
		
		if(!iterateToNextSubSet(itF1, itF2, d1WithoutD2, domain2))
			break;
		
		itRes->iterateTo(itF1->getPosition());
	}
	
	delete itF1;
	delete itF2;
	delete itRes;
	
	return result;
}

Mapping* ConstMapping::multiply(ConstMapping &f1, ConstMapping &f2)
{
	const DimensionSet& domain1 = f1.getDimensionSet();
	const DimensionSet& domain2 = f2.getDimensionSet();
	
	if(domain1.isRealSubSet(domain2)){
		return multiplySubSet(f1, f2);
	}
	
	assert(domain1 == domain2);
	
	Mapping* result = Mapping::createMapping(domain1);
	
	ConstMappingIterator* itF1 = f1.createConstIterator();
	ConstMappingIterator* itF2 = f2.createConstIterator();
	MappingIterator* itRes = result->createIterator();
	
	if(itF1->getPosition() < itF2->getPosition()){
		itF2->jumpTo(itF1->getPosition());		
	} else {
		itF1->jumpTo(itF2->getPosition());
	}
	itRes->jumpTo(itF1->getPosition());	
	
	while(itF1->inRange() || itF2->inRange()) {
		assert(itF1->getPosition().isSamePosition(itF2->getPosition()));
		
		double prod = itF1->getValue() * itF2->getValue();
		//result->setValue(itF1->getPosition(), prod);
		itRes->setValue(prod);
		
		if(!iterateToNext(itF1, itF2))
			break;
		
		itRes->iterateTo(itF1->getPosition());
	}
	
	delete itF1;
	delete itF2;
	delete itRes;
	
	return result;
}

/*Mapping* ConstMapping::multiply(ConstMapping &f1, ConstMapping &f2)
{
	DimensionSet domain1 = f1.getDimensionSet();
	DimensionSet domain2 = f2.getDimensionSet();
	
	assert(domain1.isSubSet(domain2));
	
	Mapping* result = Mapping::createMapping(domain1);
	
	ConstMappingIterator* itF1 = f1.createConstIterator();
	ConstMappingIterator* itF2 = f2.createConstIterator(itF1->getPosition());
	MappingIterator* itRes = result->createIterator(itF1->getPosition());
	
	while(itF1->inRange()) {
		assert(itF1->getPosition().isSamePosition(itF2->getPosition()));
		
		double prod = itF1->getValue() * itF2->getValue();
		//result->setValue(itF1->getPosition(), prod);
		itRes->setValue(prod);
		
		try{
			itF1->next();
		}catch(NoNextIteratorExcpetion e){
			break;
		}
		
		itF2->iterateTo(itF1->getPosition());
		itRes->iterateTo(itF1->getPosition());
	}
	
	delete itF1;
	delete itF2;
	delete itRes;
	
	return result;
}*/


//---Mapping implementation---------------------------------------

/**
 * @brief Initializes the ConstIterator for the passed ConstMapping,
 * with the passed key entries to iterate over and the passed position
 * as start. 
 * 
 * Note: The reference to the key entries has to be valid as long as the 
 * iterator exists.
 */
SimpleConstMappingIterator::SimpleConstMappingIterator(ConstMapping* mapping, 
						   const std::set<Argument>& keyEntries,
						   const Argument& start):
	mapping(mapping), 
	dimensions(mapping->getDimensionSet()),
	position(dimensions),
	keyEntries(keyEntries) {
	
	//the passed start position should define a value for every dimension
	//of this iterators underlying mapping.
	assert(start.getDimensions().isSubSet(dimensions));
	
	//Since the position is compared to the key entries we have to make
	//sure it always contains only the dimensions of the underlying mapping.
	//(the passed Argument might have more dimensions)
	position.setArgValues(start, true);
	
	nextEntry = keyEntries.upper_bound(position);
}

/**
 * @brief Initializes the ConstIterator for the passed ConstMapping,
 * with the passed key entries to iterate over.
 * 
 * Note: The reference to the key entries has to be valid as long as the 
 * iterator exists.
 */
SimpleConstMappingIterator::SimpleConstMappingIterator(ConstMapping* mapping, 
						   const std::set<Argument>& keyEntries):
	mapping(mapping), 
	dimensions(mapping->getDimensionSet()), 
	position(dimensions),
	keyEntries(keyEntries) {
	
	jumpToBegin();
}


/**
 * @brief Utility method to fill add range of key entries in the time dimension
 * to the key entry set.
 */
void SimpleConstMapping::createKeyEntries(const Argument& from, const Argument& to, const Argument& step, Argument& pos){
	
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

/**
 * @brief Utility method to fill add range of key entries in the passed dimension
 * (and recursivly its sub dimensions) to the key entry set.
 */
void SimpleConstMapping::createKeyEntries(const Argument& from, const Argument& to, const Argument& step, 
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

/**
 * @brief Initializes the key entry set with the passed min, max and 
 * interval-Arguments.
 * 
 * After a call to this method this SimpleConstMapping is able to return a valid
 * ConstIterator.
 */
void SimpleConstMapping::initializeArguments(const Argument& min,
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


Mapping* Mapping::createMapping(const DimensionSet& domain, 
							  Mapping::InterpolationMethod intpl) {
	assert(domain.hasDimension(Dimension::time));
	
	if(domain.size() == 1)
		return new TimeMapping(intpl);
	else
		return new MultiDimMapping(domain, intpl);
}


/**
 * @brief Initializes the Interator with the passed Iterators of the mappings to 
 * interpoalte and the their inteproaltionfactor.
 */
LinearIntplMappingIterator::LinearIntplMappingIterator(ConstMappingIterator* leftIt, ConstMappingIterator* rightIt, double f):
	leftIt(leftIt), rightIt(rightIt), factor(f) {
	
	assert(leftIt->getPosition() == rightIt->getPosition());
}

/**
 * @brief Deletes the left and the right mapping iterator.
 */
LinearIntplMappingIterator::~LinearIntplMappingIterator() {
	if(leftIt)
		delete leftIt;
	if(rightIt)
		delete rightIt;
}


MultiDimMappingIterator::MultiDimMappingIterator(MultiDimMapping& mapping):
	mapping(mapping), subIterator(0), subMapping(0), 
	valueIt(mapping.entries.beginIntpl()), 
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

MultiDimMappingIterator::MultiDimMappingIterator(MultiDimMapping& mapping, const Argument& pos):
	mapping(mapping), subIterator(0), subMapping(0), 
	valueIt(mapping.entries.findIntpl(pos.getArgValue(mapping.myDimension))),
	position(){
	
	subMapping = valueIt.getValue();
	if(*subMapping){
		subIterator = (*subMapping)->createIterator(pos);
	}
	
	position = pos;
	nextPosition = position;
	updateNextPosition();
	
}

void MultiDimMappingIterator::updateNextPosition() {
	if(subMapping.isInterpolated || !subIterator || !subIterator->hasNext()){
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

/**
 * @brief Lets the iterator point to the passed position.
 * 
 * The passed new position can be at arbitary places.
 */
void MultiDimMappingIterator::jumpTo(const Argument& pos) {
	double argVal = pos.getArgValue(mapping.myDimension);
	
	if(argVal != valueIt.getPosition() && pos.hasArgVal(mapping.myDimension)) {
		valueIt.jumpTo(argVal);
		updateSubIterator(pos);
	} else {
		if(subIterator)
			subIterator->jumpTo(pos);
	}

	position.setArgValues(pos);
	updateNextPosition();
}

/**
 * @brief Iterates to the specified position. This method
 * should be used if the new position is near the current position.
 */
void MultiDimMappingIterator::iterateTo(const Argument& pos) {
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
 */
void MultiDimMappingIterator::next() {
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
 * @brief Lets the iterator point to the begin of the function.
 * 
 * The beginning of the function depends on the implementation.
 */
void MultiDimMappingIterator::jumpToBegin() {
	valueIt.jumpToBegin();
	updateSubIterator();
	if(subIterator)
		position.setArgValues(subIterator->getPosition());
	
	position.setArgValue(mapping.myDimension, valueIt.getPosition());
	updateNextPosition();
}

/**
	 * @brief Changes the value of the function at the current 
 * position.
 */
void MultiDimMappingIterator::setValue(double value) {
	if(subMapping.isInterpolated) {
		valueIt.setValue(mapping.createSubSignal());
		updateSubIterator(position);
	}
	subIterator->setValue(value);
}

MultiDimMapping::MultiDimMapping(const MultiDimMapping& o):
	Mapping(o), entries(o.entries), myDimension(o.myDimension) {
	
	DimensionSet::const_iterator dimIt = dimensions.find(myDimension);
	Dimension nextDim = *(--dimIt);
	for(SubFunctionMap::iterator it = entries.begin();
		it != entries.end(); it++) {
		Mapping* tmp = it->second;
		if(nextDim == Dimension::time)
			it->second = new TimeMapping(*(static_cast<TimeMapping*>(tmp)));
		else
			it->second = new MultiDimMapping(*(static_cast<MultiDimMapping*>(tmp)));
	}
}

const MultiDimMapping& MultiDimMapping::operator=(const MultiDimMapping& o)
{
	for(SubFunctionMap::iterator it = entries.begin();
		it != entries.end(); it++) {
		
		if(it->second)
			delete it->second;
	}
	
	entries = o.entries;
	dimensions = o.dimensions;
	myDimension = o.myDimension;
	
	DimensionSet::const_iterator dimIt = dimensions.find(myDimension);
	Dimension nextDim = *(--dimIt);
	
	for(SubFunctionMap::iterator it = entries.begin();
		it != entries.end(); it++) {
		Mapping* tmp = it->second;
		if(nextDim == Dimension::time)
			it->second = new TimeMapping(*(static_cast<TimeMapping*>(tmp)));
		else
			it->second = new MultiDimMapping(*(static_cast<MultiDimMapping*>(tmp)));
	}
}
