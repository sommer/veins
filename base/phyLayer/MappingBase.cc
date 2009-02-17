#include "MappingBase.h"
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

Argument::iterator Argument::find(const Dimension& dim){
	assert(!(dim == Dimension::time));

	for(iterator it = begin(); it != end() && it->first <= dim; ++it){
		if(it->first == dim)
			return it;
	}

	return end();
}
Argument::const_iterator Argument::find(const Dimension& dim) const{
	assert(!(dim == Dimension::time));

	for(const_iterator it = begin(); it != end() && it->first <= dim; ++it){
		if(it->first == dim)
			return it;
	}

	return end();
}

Argument::iterator Argument::lower_bound(const Dimension& dim){
	assert(!(dim == Dimension::time));

	iterator it = begin();
	while(it != end() && it->first < dim)
		++it;

	return it;
}
Argument::const_iterator Argument::lower_bound(const Dimension& dim) const{
	assert(!(dim == Dimension::time));

	const_iterator it = begin();
	while(it != end() && it->first < dim)
		++it;

	return it;
}

bool Argument::hasArgVal(const Dimension& dim) const{
	return find(dim) != end();
}

double Argument::getArgValue(const Dimension & dim) const
{
	const_iterator it = find(dim);

	if(it == end())
		return double();

	return it->second;
}

void Argument::setArgValue(const Dimension & dim, double value)
{
	assert(!(dim == Dimension::time));

	insertValue(begin(), dim, value);
}

Argument::iterator Argument::insertValue(iterator pos, const Dimension& dim, double value, bool ignoreUnknown){
	while(pos != end() && !(dim < pos->first)){

		if(pos->first == dim){
			pos->second = value;
			return pos;
		}
		++pos;
	}

	if(ignoreUnknown)
		return pos;

	if(pos == end()){
		count++;
		*pos = std::pair<Dimension, double>(dim, value);
	} else {
		count++;
		iterator tmpPos = pos;
		std::pair<Dimension, double> n = *tmpPos;
		*tmpPos = std::pair<Dimension, double>(dim, value);
		while(++tmpPos != end()){
			std::pair<Dimension, double> tmp = *tmpPos;
			*tmpPos = n;
			n = tmp;
		}
	}

	return pos;
}

void Argument::setArgValues(const Argument& o, bool ingoreUnknown){
	time = o.time;

	iterator pos = begin();
	for(const_iterator i = o.begin(); i != o.end(); i++){
		pos = insertValue(pos, i->first, i->second, ingoreUnknown);
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

	const_iterator itO = o.begin();
	const_iterator it = begin();

	while (it != end())
	{
		if (itO->first < it->first) {
			break;
		} else if (it->first < itO->first)
			++it;
		else {
			//if((fabs(values[it].second - o.values[itO].second) > 0.000001)){
			if(it->second != itO->second){
				break;
			}
			++it;
			++itO;
		}
		if (itO == o.end()) return true;
	}

	return false;
}

bool Argument::isClose(const Argument& o, double epsilon) const{
	if(count != o.count)
		return false;

	if(fabs(time - o.time) > epsilon)
		return false;

	const_iterator itO = o.begin();
	for(const_iterator it = begin();
		it != end(); it++) {

		if(!(it->first == itO->first) || (fabs(it->second - itO->second) > epsilon)){
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

	const_iterator itO = o.begin();
	for(const_iterator it = begin();
		it != end(); it++) {

		if(!(it->first == itO->first) || (it->second != itO->second)){
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
		if(dim == Dimension::time)
		{
			return SIMTIME_DBL(time - o.time);
		}

		//iterate indices to the passed dimensions or the next smaller
		while(ind >= 0 && dim < values[ind].first)
			--ind;
		while(indO >= 0 && dim < o.values[indO].first)
			--indO;

		//if the last dimensions could not be found compare the time
		if(ind < 0 || indO < 0)
		{
			return SIMTIME_DBL(time - o.time);
		}

		//if both Arguments are defined in the current dimensions
		//compare them (otherwise we assume them equal and continue)
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
						   const std::set<Argument>* keyEntries,
						   const Argument& start):
	mapping(mapping),
	dimensions(mapping->getDimensionSet()),
	position(dimensions),
	keyEntries(keyEntries)
{
	assert(keyEntries);

	//the passed start position should define a value for every dimension
	//of this iterators underlying mapping.
	assert(start.getDimensions().isSubSet(dimensions));

	//Since the position is compared to the key entries we have to make
	//sure it always contains only the dimensions of the underlying mapping.
	//(the passed Argument might have more dimensions)
	position.setArgValues(start, true);

	nextEntry = keyEntries->upper_bound(position);
}

/**
 * @brief Initializes the ConstIterator for the passed ConstMapping,
 * with the passed key entries to iterate over.
 *
 * Note: The reference to the key entries has to be valid as long as the
 * iterator exists.
 */
SimpleConstMappingIterator::SimpleConstMappingIterator(ConstMapping* mapping,
						   const std::set<Argument>* keyEntries):
	mapping(mapping),
	dimensions(mapping->getDimensionSet()),
	position(dimensions),
	keyEntries(keyEntries)
{
	assert(keyEntries);

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
	for(simtime_t t = fromT; t < toT; t += stepT){
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
 * (and recursively its sub dimensions) to the key entry set.
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
	keyEntries.clear();
	DimensionSet::const_iterator dimIt = dimensions.end();
	--dimIt;
	Argument pos = min;
	if(*dimIt == Dimension::time)
		createKeyEntries(min, max, interval, pos);
	else
		createKeyEntries(min, max, interval, dimIt, pos);

	fullyInitialized = true;
}





