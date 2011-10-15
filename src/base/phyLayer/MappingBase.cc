#include "MappingBase.h"
#include <assert.h>
//---Dimension implementation-----------------------------

const Dimension Dimension::time      = Dimension::time_static();
const Dimension Dimension::frequency = Dimension::frequency_static();

Dimension::DimensionIdType& Dimension::nextFreeID () {
	static Dimension::DimensionIdType* nextID = new Dimension::DimensionIdType(1);
	return *nextID;
}

Dimension::DimensionIDMap& Dimension::dimensionIDs() {
	//use "construct-on-first-use" idiom to ensure correct order of
	//static initialization
	static DimensionIDMap* dimIDs = new DimensionIDMap();
	return *dimIDs;
}

Dimension::DimensionNameMap& Dimension::dimensionNames() {
	//use "construct-on-first-use" idiom to ensure correct order of
	//static initialization
	static DimensionNameMap* names = new DimensionNameMap();
	return *names;
}

Dimension& Dimension::time_static() {
	//use "construct-on-first-use" idiom to ensure correct order of
	//static initialization
	static Dimension* time = new Dimension("time");
	return *time;
}

Dimension& Dimension::frequency_static() {
	static Dimension* freq = new Dimension("frequency");
	return *freq;
}

Dimension::DimensionIdType Dimension::getDimensionID(const Dimension::DimensionNameType& name)
{
	//get static members one time during initialization
	static DimensionIDMap&   dimensionIDs   = Dimension::dimensionIDs();
	static DimensionIdType&  nextFreeID     = Dimension::nextFreeID();
	static DimensionNameMap& dimensionNames = Dimension::dimensionNames();

	DimensionIDMap::iterator it = dimensionIDs.lower_bound(name);

	if(it == dimensionIDs.end() || it->first != name){
		DimensionIdType newID = 0;

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

Dimension::Dimension(const Dimension::DimensionNameType& name)
	: id(getDimensionID(name))
{}

bool Dimension::operator ==(const Dimension & other) const
{
	return id == other.id;
}

bool Dimension::operator <(const Dimension & other) const
{
	return id < other.id;
}

//--DimensionSet implementation ----------------------
const DimensionSet DimensionSet::timeDomain(Dimension::time_static());
const DimensionSet DimensionSet::timeFreqDomain(Dimension::time_static(), Dimension::frequency_static());

//--Argument implementation---------------------------

Argument::Argument(simtime_t timeVal):
	time(timeVal), values(), count(0)
{}

Argument::Argument(const DimensionSet & dims, simtime_t timeVal):
	time(timeVal), values(), count(0)
{
	DimensionSet::const_iterator it = dims.begin();

	assert((*it) == Dimension::time_static());

	it++;
	while(it != dims.end()) {
		values[count] = Argument::value_type(*it, 0);
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

Argument::iterator Argument::find(const Argument::key_type& dim){
	assert(!(dim == Dimension::time_static()));

	for(iterator it = begin(); it != end() && it->first <= dim; ++it){
		if(it->first == dim)
			return it;
	}

	return end();
}
Argument::const_iterator Argument::find(const Argument::key_type& dim) const{
	assert(!(dim == Dimension::time_static()));

	for(const_iterator it = begin(); it != end() && it->first <= dim; ++it){
		if(it->first == dim)
			return it;
	}

	return end();
}

Argument::iterator Argument::lower_bound(const Argument::key_type& dim){
	assert(!(dim == Dimension::time_static()));

	iterator it = begin();
	while(it != end() && it->first < dim)
		++it;

	return it;
}
Argument::const_iterator Argument::lower_bound(const Argument::key_type& dim) const{
	assert(!(dim == Dimension::time_static()));

	const_iterator it = begin();
	while(it != end() && it->first < dim)
		++it;

	return it;
}

bool Argument::hasArgVal(const Argument::key_type& dim) const{
	return find(dim) != end();
}

double Argument::getArgValue(const Argument::key_type & dim) const
{
	const_iterator it = find(dim);

	if(it == end())
		return double();

	return it->second;
}

void Argument::setArgValue(const Argument::key_type & dim, const Argument::mapped_type& value)
{
	assert(!(dim == Dimension::time_static()));

	insertValue(begin(), dim, value);
}

Argument::iterator Argument::insertValue(iterator pos, const Argument::key_type& dim, const Argument::mapped_type& value, bool ignoreUnknown){
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
		*pos = Argument::value_type(dim, value);
	} else {
		count++;
		iterator             tmpPos = pos;
		Argument::value_type n      = *tmpPos;

		*tmpPos = Argument::value_type(dim, value);
		while(++tmpPos != end()){
			std::swap(n, *tmpPos);
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

bool Argument::isClose(const Argument& o, Argument::mapped_type epsilon) const{
	if(count != o.count)
		return false;

	if(std::abs(SIMTIME_DBL(time - o.time)) > epsilon)
		return false;

	const_iterator itO = o.begin();
	for(const_iterator it = begin();
		it != end(); it++) {

		if(!(it->first == itO->first) || (std::abs( it->second - itO->second ) > epsilon)){
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

	memcpy(values, o.values, sizeof(Argument::value_type) * count);
	//for(unsigned int i = 0; i < count; i++)
	//	values[i] = o.values[i];

	time = o.time;

}

bool Argument::operator<(const Argument & o) const
{
	assert(getDimensions() == o.getDimensions());

	return compare(o, getDimensions()) < 0;
	/*if (o.count > 0) {
		for(long it = static_cast<long>( o.count ) - 1; it >= 0; --it){
			if(values[it].second != o.values[it].second){
				return values[it].second < o.values[it].second;
			}
		}
	}
	return (time < o.time);*/
}

int Argument::compare(const Argument& o, const DimensionSet& dims) const
{
	DimensionSet::const_reverse_iterator rIt = dims.rbegin();

	long ind  = static_cast<long>(count)   - 1;
	long indO = static_cast<long>(o.count) - 1;

	//iterate through passed dimensions and compare arguments in these dimensions
	while(rIt != dims.rend()){
		const DimensionSet::value_type& dim = *rIt;

		//catch special case time (after which we can abort)
		if(dim == Dimension::time) {
			if (time == o.time)
				return 0;
			return (time < o.time) ? -1 : 1;
		}

		//iterate indices to the passed dimensions or the next smaller
		while(ind  >= 0 && dim < values[ind].first)
			--ind;
		while(indO >= 0 && dim < o.values[indO].first)
			--indO;

		//if the last dimensions could not be found compare the time
		if(ind < 0 || indO < 0) {
			if (time == o.time)
				return 0;
			return (time < o.time) ? -1 : 1;
		}

		//if both Arguments are defined in the current dimensions
		//compare them (otherwise we assume them equal and continue)
		if(values[ind].first == dim && o.values[indO].first == dim){
			if (values[ind].second != o.values[indO].second)
				return (values[ind].second < o.values[indO].second) ? -1 : 1;
		}
		++rIt;
	}
	return 0;
}

//---Mapping implementation---------------------------------------

SimpleConstMappingIterator::SimpleConstMappingIterator(ConstMapping*                                  mapping,
                                                       const SimpleConstMappingIterator::KeyEntrySet* keyEntries,
                                                       const Argument&                                start)
	: mapping(mapping)
	, dimensions(mapping->getDimensionSet())
	, position(dimensions)
	, keyEntries(keyEntries)
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

SimpleConstMappingIterator::SimpleConstMappingIterator(ConstMapping*                                  mapping,
                                                       const SimpleConstMappingIterator::KeyEntrySet* keyEntries)
	: mapping(mapping)
	, dimensions(mapping->getDimensionSet())
	, position(dimensions)
	, keyEntries(keyEntries)
{
	assert(keyEntries);

	jumpToBegin();
}

void SimpleConstMapping::createKeyEntries(const Argument& from, const Argument& to, const Argument& step, Argument& pos)
{
	//get iteration borders and steps
	const simtime_t& fromT = from.getTime();
	const simtime_t& toT   = to.getTime();
	const simtime_t& stepT = step.getTime();

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

void SimpleConstMapping::createKeyEntries(const Argument& from, const Argument& to, const Argument& step,
                                          DimensionSet::const_iterator curDim, Argument& pos)
{
	//get the dimension to iterate over
	DimensionSet::value_type d = *curDim;

	//increase iterator to next dimension (means curDim now stores the next dimension)
	--curDim;
	bool nextIsTime = (*curDim == Dimension::time_static());

	//get our iteration borders and steps
	const Argument::mapped_type& fromD = from.getArgValue(d);
	const Argument::mapped_type& toD   = to.getArgValue(d);
	const Argument::mapped_type& stepD = step.getArgValue(d);

	//iterate over interval without the last entry
	for(Argument::mapped_type i = fromD; i < toD; i += stepD){
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
}





