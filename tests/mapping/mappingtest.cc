#include <Mapping.h>
#include <iostream>
#include <sstream>
#include <string>
#include "Time.h"
#include "../testUtils/asserts.h"



void testDimension() {
	
	Dimension d1("time");	
	Dimension d2("frequency");	
	
	assertEqual("Check time dimensions name.", "time", d1.getName());
	assertEqual("Check freq dimensions name.", "frequency", d2.getName());
	assertTrue("Time dimension should be smaller then frequency.", d1 < d2);
	assertFalse("Time dimension should not be equal with frequency.", d1 == d2);
	
	assertEqual("With \"time\" created dimension should be equal with Dimension::time", Dimension::time, d1);
	
	Dimension d3("time");
	assertEqual("Check time2 dimensions name.", "time", d3.getName());
	assertTrue("Copy of time dimension should be equal with original.", d1 == d3);
	assertFalse("Copy of time dimension should not be smaller than original.", d1 < d3);
	
	Dimension d4("space");	
	assertEqual("Check space dimensions name.", "space", d4.getName());
	assertFalse("Space dimension should not be smaller then frequency.", d4 < d2);
	
	DimensionSet dims;
	dims.addDimension(d2);
	assertEqual("first dimension should be freq.", d2, dims.getFirst());	
	assertEqual("next dimension of freq should be freq.", d2, dims.getNext(d2));
	
	dims.addDimension(d4);
	assertEqual("first dimension should be space.", d4, dims.getFirst());	
	assertEqual("next dimension of freq should be freq.", d2, dims.getNext(d2));
	assertEqual("next dimension of space should be freq.", d2, dims.getNext(d4));
}


template<class T>
void assertEqualNotSmaller(std::string msg, T& v1, T& v2){
	assertEqual(msg, v1, v2);
	assertFalse(msg, v1 < v2);
	assertFalse(msg, v2 < v1);
	assertEqual(msg, 0, v1.compare(v2, v1.getDimensions()));
}

void testArg() {
	Dimension time("time");
	Argument a1(10.2);
	
	assertClose("Check initial time value of a1.", 10.2, a1.getTime());
	
	a1.setTime(-4.2);
	assertEqualSilent("Check time value of a1 after setTimeValue.", -4.2, a1.getTime());
	
	Argument a2(-4.2);	
	assertEqualNotSmaller("a1 and a2 should be equal.", a1, a2);
	
	a2.setTime(-4.3);
	assertTrue("a2 with smaller time should be smaller than a1", a2 < a1);
	assertTrue("a2 with smaller time should be compared smaller than a1", a2.compare(a1, a1.getDimensions()) < 0);
	
	a2.setTime(0.0);
	assertTrue("a1 with smaller time should be smaller than a2", a1 < a2);
	assertTrue("a1 with smaller time should be compared smaller than a2", a1.compare(a2, a1.getDimensions()) < 0);
	
	Dimension freq("frequency");
		
	a1.setArgValue(freq, 2.5);
	assertEqualSilent("time dimension should still have same value.", -4.2, a1.getTime());
	assertEqualSilent("Check frequency dimension value.", 2.5, a1.getArgValue(freq));
	
	a2.setTime(-4.2);
	
	//assertFalse("a1 and a2 with same time and implicit same freq should not be smaller.", a1 < a2);
	assertTrue("a1 and a2 with same time and implicit same freq should be same.", a1.isSamePosition(a2));
	assertFalse("a1 and a2 with same time and implicit same freq should not be equal.", a1 == a2);
	assertEqual("a1 and a2 with same time and implicit same freq should be compared same.", 0, a1.compare(a2, a2.getDimensions()));
	
	a1.setArgValue(freq, -2.2);
	//assertFalse("a1 and a2 with same time and implicit same freq should still not be smaller.", a1 < a2);
	assertEqual("a1 and a2 with same time and implicit same freq should still be compared same.", 0, a1.compare(a2, a2.getDimensions()));
	assertTrue("a1 and a2 with same time and implicit same freq should still be same.", a1.isSamePosition(a2));
	assertFalse("a1 and a2 with same time and implicit same freq should still not be equal.", a1 == a2);
	
	a2.setTime(-5);
	//assertFalse("a1 with bigger time and implicit equal freq should not be smaller.", a1 < a2);
	assertTrue("a1 with bigger time and implicit equal freq should be compared bigger.", a1.compare(a2, a2.getDimensions()) > 0);
	assertFalse("a1 with bigger time and implicit equal freq should not be same.", a1.isSamePosition(a2));
	assertFalse("a1 with bigger time and implicit equal freq should not be equal.", a1 == a2);
	
	a1.setTime(-6);
	//assertTrue("a1 with smaller time and implicit equal freq should be smaller.", a1 < a2);
	assertTrue("a1 with smaller time and implicit equal freq should be compared smaller.", a1.compare(a2, a2.getDimensions()) < 0);
	assertFalse("a1 with smaller time and implicit equal freq should not be same.", a1.isSamePosition(a2));
	assertFalse("a1 with smaller time and implicit equal freq should not be equal.", a1 == a2);
	
	a1.setTime(-4.2);
	a1.setArgValue(freq, 2.5);	
	a2.setTime(-4.2);
	a2.setArgValue(freq, 2.5);
	
	assertEqual("a1 and a2 with same time and freq should be equal.", a1, a2);
	assertEqual("a1 and a2 with same time and freq should be compared equal.", 0, a1.compare(a2, a1.getDimensions()));
	
	a2.setTime(-4.3);
	assertTrue("a2 with smaller time and same freq should be smaller than a1", a2 < a1);
	assertTrue("a2 with smaller time and same freq should be compared smaller than a1", a2.compare(a1, a1.getDimensions()) < 0);
	assertFalse("a2 with smaller time and same freq should not be equal with a1.", a1 == a2);
	
	a2.setTime(0.0);
	assertTrue("a1 with smaller time and same freq should be compared smaller than a2", a1.compare(a2, a2.getDimensions()) < 0);
	assertTrue("a1 with smaller time and same freq should be smaller than a2", a1 < a2);
	assertFalse("a1 with smaller time and same freq should not be equal with a2.", a1 == a2);
	
	a2.setTime(-4.2);
	a2.setArgValue(freq, 2.0);
	assertTrue("a2 with smaller freq should be smaller than a1", a2.compare(a1, a1.getDimensions()) < 0);
	assertTrue("a2 with smaller freq should be smaller than a1", a2 < a1);
	assertFalse("a2 with smaller freq should not be equal with a1.", a1 == a2);
	
	a2.setArgValue(freq, 3.0);
	assertTrue("a1 with smaller freq should be smaller than a2", a1.compare(a2, a2.getDimensions()) < 0);
	assertTrue("a1 with smaller freq should be smaller than a2", a1 < a2);
	assertFalse("a1 with smaller freq should not be equal with a2.", a1 == a2);
	
	a2.setTime(-20.0);
	assertTrue("a1 with smaller freq should still be smaller than a2 with smaller time", a2.compare(a1, a2.getDimensions()) > 0);
	assertTrue("a1 with smaller freq should still be smaller than a2 with smaller time", a1 < a2);
	assertFalse("a1 with smaller freq should not be equal with a2 with smaller time.", a1 == a2);
	
	a2.setTime(40.0);
	a2.setArgValue(freq, 2.2);
	assertTrue("a2 with smaller freq should still be smaller than a1 with smaller time", a1.compare(a2, a2.getDimensions()) > 0);
	assertTrue("a2 with smaller freq should still be smaller than a1 with smaller time", a2 < a1);
	assertFalse("a2 with smaller freq should not be equal with a1 with smaller time.", a1 == a2);
	
	displayPassed = false;
}

void assertClose(std::string msg, Argument target, Argument actual){
	if(actual.isClose(target)){
		if(displayPassed)
			std::cout << "Passed: " << msg << "\n";
	} else {
		std::cout << "FAILED: " << msg << ": expected " << target << " was " << actual << "\n";
	}
}

void checkIterator(std::string msg, ConstMappingIterator& it, 
				   bool hasNext, bool inRange, 
				   Argument arg, Argument nextArg, 
				   double val, ConstMapping& f) {
	assertEqual(msg + ": hasNext() at " + toString(arg), hasNext, it.hasNext());
	assertEqual(msg + ": inRange() at " + toString(arg), inRange, it.inRange());
	assertClose(msg + ": currentPos() at " + toString(arg), arg, it.getPosition());
	try{
		assertClose(msg + ": nextPos() at " + toString(arg), nextArg, it.getNextPosition());
	}catch(NoNextIteratorExcpetion e){
		assertFalse("HasNext should be false on NoNextException.", hasNext);
	}
	assertClose(msg + ": getValue() at " + toString(arg), val, it.getValue());
	assertClose(msg + ": Equal with function at " + toString(arg), f.getValue(it.getPosition()), it.getValue());
}

void checkNext(std::string msg, ConstMappingIterator& it, 
			   bool hasNext, bool inRange, 
			   Argument arg, Argument nextArg,
			   double val, ConstMapping& f) {
	it.next();
	checkIterator(msg, it, hasNext, inRange, arg, nextArg, val, f);
}

template<class F>
void testSimpleFunction() {
	Dimension time("time");
	F f;
	Argument a3(3.11);
	
	assertEqualSilent("Function should be zero initially.", 0.0, f.getValue(a3));
	assertEqualSilent("Function should be zero initially (with []).", 0.0, f[a3]);
	
	f.setValue(a3, 5.0);
	assertEqualSilent("Check function value at a1.", 5.0, f[a3]);
	
	Argument a4(4.11);
	assertEqualSilent("Linear function should be constant after single value.", 5.0, f[a4]);
	Argument a2(2.01);
	assertEqualSilent("Linear function should beconstant before single value.", 5.0, f[a2]);
	
	f.setValue(a4, 10.0);
	assertEqualSilent("Check new set value at a2.", 10.0, f[a4]);
	assertEqualSilent("Check if still same value at a1.", 5.0, f[a3]);
	assertEqualSilent("Linear function should beconstant before first value.", 5.0, f[a2]);
	
	Argument a5(5.0);
	assertEqualSilent("Linear function should beconstant after last value.", 10.0, f[a5]);
	
	Argument halfFirst(3.61);
	assertClose("CHeck interpoaltion at half way.", 7.5, f[halfFirst]);
	assertClose("CHeck interpoaltion at quarter way.", 6.25, f[Argument(3.36)]);
	
	f.setValue(a5, 7.0);
	
	assertEqualSilent("Check new set value at a5.", 7.0, f[a5]);
	assertEqualSilent("Check if still same value at a1.", 5.0, f[a3]);
	assertEqualSilent("Linear function should beconstant before first value.", 5.0, f[a2]);
	
	Argument a6(6.0);
	assertEqualSilent("Linear function should beconstant after last value.", 7.0, f[a6]);
	
	Argument halfSecond(4.555);
	assertClose("CHeck interpoaltion at half way.", 8.5, f[halfSecond]);	
	
	F emptyF;
	
	MappingIterator* it = emptyF.createIterator();	
	checkIterator("Empty Iterator initial", *it, false, false, Argument(0.0), Argument(1.0), 0.0, emptyF);
	checkNext("First next of Empty iterator", *it, false, false, Argument(1.0), Argument(2.0), 0.0, emptyF);
	
	it->iterateTo(a6);
	checkIterator("Empty Iterator iterateTo()", *it, false, false, a6, Argument(7.0), 0.0, emptyF);
	
	it->jumpTo(a2);
	checkIterator("Empty Iterator jumpTo(back)", *it, false, false, a2, Argument(3.01), 0.0, emptyF);
	
	it->jumpTo(a5);
	checkIterator("Empty Iterator jumpTo(forward)", *it, false, false, a5, Argument(6.0), 0.0, emptyF);
	
	it->setValue(6.0);
	checkIterator("Set empty iterator", *it, false, true, a5, Argument(6.0), 6.0, emptyF);
	
	delete it;
		
	it = f.createIterator();	
	checkIterator("Initial iterator", *it, true, true, a3, a4, 5.0, f);
	
	it->setValue(6.0);
	checkIterator("Set Initial iterator", *it, true, true, a3, a4, 6.0, f);
	
	it->iterateTo(halfFirst);
	checkIterator("First half iterator", *it, true, true, halfFirst, a4, 8.0, f);
	
	it->setValue(7.5);
	checkIterator("Set first half iterator", *it, true, true, halfFirst, a4, 7.5, f);
	
	checkNext("Second (after first half) iterator", *it, true, true, a4, a5, 10.0, f);
	checkNext("Last iterator", *it, false, true, a5, Argument(6.0), 7.0, f);	
	
	Argument prevAll(0.1);
	it->jumpTo(prevAll);
	checkIterator("Prev all", *it, true, false, prevAll, a3, 6.0, f);
	
	it->jumpTo(Argument(0.2));
	checkIterator("Second prev all", *it, true, false, Argument(0.2), a3, 6.0, f);
	
	it->setValue(3.0);
	checkIterator("Set second prev all", *it, true, true, Argument(0.2), a3, 3.0, f);
	
	checkNext("Next of Prev all (first)", *it, true, true, a3, halfFirst, 6.0, f);
	
	it->iterateTo(a6);
	checkIterator("After all", *it, false, false, a6, Argument(7.0), 7.0, f);
	
	it->setValue(3.0);
	checkIterator("Set after all", *it, false, true, a6, Argument(7.0), 3.0, f);
	
	delete it;
	
	it = f.createIterator();
	checkIterator("Total 1:", *it, true, true, Argument(0.2), a3, 3.0, f);
	checkNext("Total 2:", *it, true, true, a3, halfFirst, 6.0, f);
	checkNext("Total 3:", *it, true, true, halfFirst, a4, 7.5, f);
	checkNext("Total 4:", *it, true, true, a4, a5, 10.0, f);
	checkNext("Total 5:", *it, true, true, a5, a6, 7.0, f);
	checkNext("Total 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0, f);
	checkNext("Total 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0, f);
	delete it;
	
	
	F f2;
	f2.setValue(Argument(3.86), 1.0);
	
	Mapping* res = f * f2;	
		
	it = res->createIterator();
	checkIterator("f*1 1:", *it, true, true, Argument(0.2), a3, 3.0, *res);
	checkNext("f*1 2:", *it, true, true, a3, halfFirst, 6.0, *res);
	checkNext("f*1 3:", *it, true, true, halfFirst, Argument(3.86), 7.5, *res);
	checkNext("f*1 3.5:", *it, true, true, Argument(3.86), a4, 8.75, *res);
	checkNext("f*1 4:", *it, true, true, a4, a5, 10.0, *res);
	checkNext("f*1 5:", *it, true, true, a5, a6, 7.0, *res);
	checkNext("f*1 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0, *res);
	checkNext("f*1 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0, *res);
	delete it;
	delete res;
	
	F f2b;
	f2b.setValue(Argument(0.0), 2.0);
		
	res = f * f2b;	
		
	it = res->createIterator();
	checkIterator("f*2 0:", *it,true, true, Argument(0.0), Argument(0.2), 6.0, *res);
	checkNext("f*2 1:", *it,true, true, Argument(0.2), a3, 6.0, *res);
	checkNext("f*2 2:", *it, true, true, a3, halfFirst, 12.0, *res);
	checkNext("f*2 3:", *it, true, true, halfFirst, a4, 15.0, *res);
	checkNext("f*2 4:", *it, true, true, a4, a5, 20.0, *res);
	checkNext("f*2 5:", *it, true, true, a5, a6, 14.0, *res);
	checkNext("f*2 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 6.0, *res);
	checkNext("f*2 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 6.0, *res);
	delete it;
	delete res;
	
	F f3(f);
	
	res = f * f3;	
			
	it = res->createIterator();
	checkIterator("f^2 1:", *it, true, true, Argument(0.2), a3, 3.0 * 3.0, *res);
	checkNext("f^2 2:", *it, true, true, a3, halfFirst, 6.0 * 6.0, *res);
	checkNext("f^2 3:", *it, true, true, halfFirst, a4, 7.5 * 7.5, *res);
	checkNext("f^2 4:", *it, true, true, a4, a5, 10.0 * 10.0, *res);
	checkNext("f^2 5:", *it, true, true, a5, a6, 7.0 * 7.0, *res);
	checkNext("f^2 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0 * 3.0, *res);
	checkNext("f^2 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0 * 3.0, *res);
	delete it;
	delete res;
	
	F f4;
	
	f4.setValue(Argument(0.2), 1.0);
	f4.setValue(a6, 2.0);
	
	res = f3 * f4;
	double fak = 1.0 / (a6.getTime() - 0.2);
	it = res->createIterator();
	checkIterator("f*1-2 1:", *it, true, true, Argument(0.2), a3, 3.0 * 1.0, *res);
	checkNext("f*1-2 2:", *it, true, true, a3, halfFirst, 6.0 * (1.0 + (a3.getTime() - 0.2) * fak), *res);
	checkNext("f*1-2 3:", *it, true, true, halfFirst, a4, 7.5 * (1.0 + (halfFirst.getTime() - 0.2) * fak), *res);
	checkNext("f*1-2 4:", *it, true, true, a4, a5, 10.0 * (1.0 + (a4.getTime() - 0.2) * fak), *res);
	checkNext("f*1-2 5:", *it, true, true, a5, a6, 7.0 * (1.0 + (a5.getTime() - 0.2) * fak), *res);
	checkNext("f*1-2 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0 * (1.0 + (a6.getTime() - 0.2) * fak), *res);
	checkNext("f*1-2 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0 * (1.0 + (a6.getTime() - 0.2) * fak), *res);
	delete it;
	delete res;
}

void testMultiFunction() {
	Dimension time("time");
	Dimension channel("channel");
	
	DimensionSet dimSet(Dimension::time);
	dimSet.addDimension(channel);
	
	MultiDimMapping f(dimSet);
	assertEqualSilent("Dimension of f", channel, f.getDimension());
	
	std::map<double, std::map<double, Argument> > a;
		
	for(double i = 0.0; i <= 6.0; i+=0.25) {
		for(double j = 0.0; j <= 6.0; j+=0.25) {
			a[i][j].setTime(j);
			a[i][j].setArgValue(channel, i);
		}
	}
		
	assertEqual("Function should be zero initially.", 0.0, f.getValue(a[2][2]));
	assertEqual("Function should be zero initially (with []).", 0.0, f[a[2][2]]);
	
	f.setValue(a[2][2], 5.0);
	for(double i = 1.0; i <= 4.0; i+=1.0) {
		for(double j = 1.0; j <= 4.0; j+=1.0) {
			assertEqual("Function should be constant 5 at "
						+ toString(i) + "," + toString(j), 5.0, f.getValue(a[i][j]));
		}
	}
	
	f.setValue(a[3][2], 10.0);
	for(double i = 1.0; i <= 4.0; i+=0.5) {
		for(double j = 1.0; j <= 4.0; j+=0.5) {
			if(i <= 2.0)
				assertEqual("Function should be constant 5 before time 2 at "
							+ toString(i) + "," + toString(j), 5.0, f.getValue(a[i][j]));
			
			else if(i >= 3.0)
				assertEqual("Function should be constant 10 after time 3 at "
							+ toString(i) + "," + toString(j), 10.0, f.getValue(a[i][j]));
			
			else
				assertEqual("Function should be constant interpolated between time 2 and 3 at "
							+ toString(i) + "," + toString(j), 7.5, f.getValue(a[i][j]));
		}
	}
	
	MultiDimMapping f2(dimSet);
	f2.setValue(a[2][2], 5.0);
	f2.setValue(a[2][3], 10.0);
	for(double i = 1.0; i <= 4.0; i+=0.5) {
		for(double j = 1.0; j <= 4.0; j+=0.5) {
			if(j <= 2.0)
				assertEqual("Function should be constant 5 before time 2 at "
							+ toString(i) + "," + toString(j), 5.0, f2.getValue(a[i][j]));
			
			else if(j >= 3.0)
				assertEqual("Function should be constant 10 after time 3 at "
							+ toString(i) + "," + toString(j), 10.0, f2.getValue(a[i][j]));
			
			else
				assertEqual("Function should be constant interpolated between time 2 and 3 at "
							+ toString(i) + "," + toString(j), 7.5, f2.getValue(a[i][j]));
		}
	}
	
	f2.setValue(a[3][2], 50.0);
	f2.setValue(a[3][3], 100.0);
	for(double i = 1.0; i <= 4.0; i+=0.5) {
		for(double j = 1.0; j <= 4.0; j+=0.5) {
			if(j <= 2.0){
				if(i <= 2.0)
					assertEqual("Function should be constant 5"
								+ toString(i) + "," + toString(j), 5.0, f2.getValue(a[i][j]));
				else if(i >= 3.0)
					assertEqual("Function should be constant 50"
								+ toString(i) + "," + toString(j), 50.0, f2.getValue(a[i][j]));
				else
					assertEqual("Function should be constant 27.5 "
								+ toString(i) + "," + toString(j), 27.5, f2.getValue(a[i][j]));
			
			}else if(j >= 3.0){
				if(i <= 2.0)
					assertEqual("Function should be constant 10"
								+ toString(i) + "," + toString(j), 10.0, f2.getValue(a[i][j]));
				else if(i >= 3.0)
					assertEqual("Function should be constant 100"
								+ toString(i) + "," + toString(j), 100.0, f2.getValue(a[i][j]));
				else
					assertEqual("Function should be constant 55 "
								+ toString(i) + "," + toString(j), 55.0, f2.getValue(a[i][j]));
			
			}else{
				if(i <= 2.0)
					assertEqual("Function should be constant 7.5"
								+ toString(i) + "," + toString(j), 7.5, f2.getValue(a[i][j]));
				else if(i >= 3.0)
					assertEqual("Function should be constant 75"
								+ toString(i) + "," + toString(j), 75.0, f2.getValue(a[i][j]));
				else
					assertEqual("Function should be constant 41.25 "
								+ toString(i) + "," + toString(j), 41.25, f2.getValue(a[i][j]));
			}
		}
	}
	
	MultiDimMapping fEmpty(dimSet);
	MappingIterator* it = fEmpty.createIterator();
	
	checkIterator("Empty Iterator initial", *it, false, false, a[0][0], a[1][0], 0.0, fEmpty);
	checkNext("First next of Empty iterator", *it, false, false, a[1][0], a[2][0], 0.0, fEmpty);
	
	it->iterateTo(a[2][3]);
	checkIterator("Empty Iterator iterateTo()", *it, false, false, a[2][3], a[3][3], 0.0, fEmpty);
	
	it->jumpTo(a[1][1]);
	checkIterator("Empty Iterator jumpTo(back)", *it, false, false, a[1][1], a[2][1], 0.0, fEmpty);
	
	it->jumpTo(a[3][4]);
	checkIterator("Empty Iterator jumpTo(forward)", *it, false, false, a[3][4], a[4][4], 0.0, fEmpty);
	
	it->setValue(6.0);
	checkIterator("Set empty iterator", *it, false, true, a[3][4], a[4][4], 6.0, fEmpty);
	
	delete it;
	
	it = f2.createIterator();
	it->jumpTo(a[1.0][1.0]);
	for(double i = 1.0; i <= 4.0; i+=0.5) {
		for(double j = 1.0; j <= 4.0; j+=0.5) {
			it->iterateTo(a[i][j]);
			if(j <= 2.0){
				if(i <= 2.0)
					assertEqual("Function should be constant 5"
								+ toString(i) + "," + toString(j), 5.0, it->getValue());
				else if(i >= 3.0)
					assertEqual("Function should be constant 50"
								+ toString(i) + "," + toString(j), 50.0, it->getValue());
				else
					assertEqual("Function should be constant 27.5 "
								+ toString(i) + "," + toString(j), 27.5, it->getValue());
			
			}else if(j >= 3.0){
				if(i <= 2.0)
					assertEqual("Function should be constant 10"
								+ toString(i) + "," + toString(j), 10.0, it->getValue());
				else if(i >= 3.0)
					assertEqual("Function should be constant 100"
								+ toString(i) + "," + toString(j), 100.0, it->getValue());
				else
					assertEqual("Function should be constant 55 "
								+ toString(i) + "," + toString(j), 55.0, it->getValue());
			
			}else{
				if(i <= 2.0)
					assertEqual("Function should be constant 7.5"
								+ toString(i) + "," + toString(j), 7.5, it->getValue());
				else if(i >= 3.0)
					assertEqual("Function should be constant 75"
								+ toString(i) + "," + toString(j), 75.0, it->getValue());
				else
					assertEqual("Function should be constant 41.25 "
								+ toString(i) + "," + toString(j), 41.25, it->getValue());
			}			
		}
	}
	
	delete it;
	
	it = f2.createIterator();
	checkIterator("Check initial MultiDimFunction Iterator22", *it, true, true, a[2][2], a[2][3], 5.0, f2);
	it->setValue(6.0);
	checkIterator("Set Initial iterator22", *it, true, true, a[2][2], a[2][3], 6.0, f2);
	
	it->iterateTo(a[2][2.5]);
	checkIterator("First half iterator", *it, true, true, a[2][2.5], a[2][3], 8.0, f2);
	it->setValue(7.5);
	checkIterator("Set first half iterator", *it, true, true, a[2][2.5], a[2][3], 7.5, f2);
			
	checkNext("First next of MultiDimFunction23", *it, true, true, a[2][3], a[3][2], 10.0, f2);
	
	it->iterateTo(a[2.5][1.5]);
	checkIterator("second half iterator", *it, true, true, a[2.5][1.5], a[3][2], 28.0, f2);
	it->setValue(8.0);
	checkIterator("Set second half iterator", *it, true, true, a[2.5][1.5], a[3][2], 8.0, f2);
	
	it->iterateTo(a[2.75][2.5]);
	checkIterator("Third half iterator", *it, true, true, a[2.75][2.5], a[3][2], 41.5, f2);
		
	checkNext("Second next of MultiDimFunction32", *it, true, true, a[3][2], a[3][3], 50.0, f2);
	checkNext("Third next of MultiDimFunction33", *it, false, true, a[3][3], a[4][3], 100.0, f2);
	it->next();
	assertEqual("Iterator after end43: hasNext()", false, it->hasNext());
	assertEqual("Iterator after end43: inRange()", false, it->inRange());
	assertTrue("Iterator after end43: currentPos() not smaller last pos", !(it->getPosition() < a[3][3]));
	assertClose("Iterator after end43: Equal with function", f2.getValue(it->getPosition()), it->getValue());
	
	
	it->jumpTo(a[1][1]);
	checkIterator("Prev all", *it, true, false, a[1][1], a[2][2], 6.0, f2);
		
	it->jumpTo(a[1.5][1]);
	checkIterator("Second prev all", *it, true, false, a[1.5][1], a[2][2], 6.0, f2);		
	it->setValue(3.0);
	checkIterator("Set second prev all", *it, true, true, a[1.5][1], a[2][2], 3.0, f2);
		
	checkNext("Next of Prev all (first)", *it, true, true, a[2][2], a[2][2.5], 6.0, f2);
		
	it->iterateTo(a[3.5][3.5]);
	checkIterator("After all", *it, false, false, a[3.5][3.5], a[4.5][3.5], 100.0, f2);		
	it->setValue(3.0);
	checkIterator("Set after all", *it, false, true, a[3.5][3.5], a[4.5][3.5], 3.0, f2);
	
	delete it;
	
	
	//Multiplikation
	
	f = MultiDimMapping(f2);
	
	it = f.createIterator();
	checkIterator("Total 1:", *it, true, true, a[1.5][1], a[2][2], 3.0, f);
	checkNext("Total 2:", *it, true, true, a[2][2], a[2][2.5], 6.0, f);
	checkNext("Total 3:", *it, true, true, a[2][2.5], a[2][3], 7.5, f);
	checkNext("Total 4:", *it, true, true, a[2][3], a[2.5][1.5], 10.0, f);
	checkNext("Total 5:", *it, true, true, a[2.5][1.5], a[3][2], 8.0, f);
	checkNext("Total 6:", *it, true, true, a[3][2], a[3][3], 50.0, f);
	checkNext("Total 7:", *it, true, true, a[3][3], a[3.5][3.5], 100.0, f);
	checkNext("Total 8:", *it, false, true, a[3.5][3.5], a[4.5][3.5], 3.0, f);
	checkNext("Total 9:", *it, false, false, a[4.5][3.5], a[5.5][3.5], 3.0, f);
	delete it;
	
	
	
	TimeMapping simpleConstTime;
	simpleConstTime.setValue(Argument(1.0), 1.0);
	
	Mapping* res = f * simpleConstTime;	
	
	displayPassed = false;
		
	it = res->createIterator();
	checkIterator("f*1 1:", *it, true, true, a[1.5][1], a[2][1], 3.0, *res);
	checkNext("f*1 1.5:", *it, true, true, a[2][1], a[2][2], 6.0, *res);
	checkNext("f*1 2:", *it, true, true, a[2][2], a[2][2.5], 6.0, *res);
	checkNext("f*1 3:", *it, true, true, a[2][2.5], a[2][3], 7.5, *res);
	checkNext("f*1 4:", *it, true, true, a[2][3], a[2.5][1], 10.0, *res);
	checkNext("f*1 4.5:", *it, true, true, a[2.5][1], a[2.5][1.5], 8.0, *res);
	checkNext("f*1 5:", *it, true, true, a[2.5][1.5], a[3][1], 8.0, *res);
	checkNext("f*1 5.5:", *it, true, true, a[3][1], a[3][2], 50.0, *res);
	checkNext("f*1 6:", *it, true, true, a[3][2], a[3][3], 50.0, *res);
	checkNext("f*1 7:", *it, true, true, a[3][3], a[3.5][1], 100.0, *res);
	checkNext("f*1 7.5:", *it, true, true, a[3.5][1], a[3.5][3.5], 3.0, *res);
	checkNext("f*1 8:", *it, false, true, a[3.5][3.5], a[4.5][3.5], 3.0, *res);
	checkNext("f*1 9:", *it, false, false, a[4.5][3.5], a[5.5][3.5], 3.0, *res);
	delete it;
	delete res;
	
	displayPassed = false;
	
	
	simpleConstTime.setValue(Argument(1.0), 2.0);
		
	res = f * simpleConstTime;	
		
	it = res->createIterator();
	checkIterator("f*2 1:", *it, true, true, a[1.5][1], a[2][1], 6.0, *res);
	checkNext("f*2 1.5:", *it, true, true, a[2][1], a[2][2], 12.0, *res);
	checkNext("f*2 2:", *it, true, true, a[2][2], a[2][2.5], 12.0, *res);
	checkNext("f*2 3:", *it, true, true, a[2][2.5], a[2][3], 15.0, *res);
	checkNext("f*2 4:", *it, true, true, a[2][3], a[2.5][1], 20.0, *res);
	checkNext("f*2 4.5:", *it, true, true, a[2.5][1], a[2.5][1.5], 16.0, *res);
	checkNext("f*2 5:", *it, true, true, a[2.5][1.5], a[3][1], 16.0, *res);
	checkNext("f*2 5.5:", *it, true, true, a[3][1], a[3][2], 100.0, *res);
	checkNext("f*2 6:", *it, true, true, a[3][2], a[3][3], 100.0, *res);
	checkNext("f*2 7:", *it, true, true, a[3][3], a[3.5][1], 200.0, *res);
	checkNext("f*2 7.5:", *it, true, true, a[3.5][1], a[3.5][3.5], 6.0, *res);
	checkNext("f*2 8:", *it, false, true, a[3.5][3.5], a[4.5][3.5], 6.0, *res);
	checkNext("f*2 9:", *it, false, false, a[4.5][3.5], a[5.5][3.5], 6.0, *res);
	delete it;
	delete res;
	
	assertEqualSilent("Dimension of f", channel, f.getDimension());
	MultiDimMapping f3(f);
	assertEqualSilent("Dimension of f", channel, f.getDimension());
	assertEqualSilent("Dimension of f3", channel, f3.getDimension());
	
	res = f * f3;	
			
	it = res->createIterator();
	checkIterator("f^2 1:", *it, true, true, a[1.5][1], a[2][2], 3.0 * 3.0, *res);
	checkNext("f^2 2:", *it, true, true, a[2][2], a[2][2.5], 6.0 * 6.0, *res);
	checkNext("f^2 3:", *it, true, true, a[2][2.5], a[2][3], 7.5 * 7.5, *res);
	checkNext("f^2 4:", *it, true, true, a[2][3], a[2.5][1.5], 10.0 * 10.0, *res);
	checkNext("f^2 5:", *it, true, true, a[2.5][1.5], a[3][2], 8.0 * 8.0, *res);
	checkNext("f^2 6:", *it, true, true, a[3][2], a[3][3], 50.0 * 50.0, *res);
	checkNext("f^2 7:", *it, true, true, a[3][3], a[3.5][3.5], 100.0 * 100.0, *res);
	checkNext("f^2 8:", *it, false, true, a[3.5][3.5], a[4.5][3.5], 3.0 * 3.0, *res);
	checkNext("f^2 9:", *it, false, false, a[4.5][3.5], a[5.5][3.5], 3.0 * 3.0, *res);
	delete it;
	delete res;
	
	
	TimeMapping f4;
	
	f4.setValue(Argument(1.0), 1.0);
	f4.setValue(Argument(3.5), 1.5);
	f4.setValue(Argument(6.0), 2.0);
		
	assertEqualSilent("Dimension of f3", channel, f3.getDimension());
	res = f3 * f4;
	double fak = 1.0 / 5.0;
	it = res->createIterator();
	checkIterator("f*x 1:", *it, true, true, a[1.5][1], a[1.5][3.5], 3.0  * (1.0 + 0 * fak), *res);
	checkNext("f*x 1.1:", *it, true, true, a[1.5][3.5], a[1.5][6], 3.0  * (1.0 + 2.5 * fak), *res);
	checkNext("f*x 1.2:", *it, true, true, a[1.5][6], a[2][1], 3.0  * (1.0 + 5.0 * fak), *res);
	
	checkNext("f*x 1.3:", *it, true, true, a[2][1], a[2][2], 6.0  * (1.0 + 0.0 * fak), *res);
	checkNext("f*x 2:", *it, true, true, a[2][2], a[2][2.5], 6.0  * (1.0 + 1.0 * fak), *res);
	checkNext("f*x 3:", *it, true, true, a[2][2.5], a[2][3], 7.5  * (1.0 + 1.5 * fak), *res);
	checkNext("f*x 4:", *it, true, true, a[2][3], a[2][3.5], 10.0  * (1.0 + 2.0 * fak), *res);
	checkNext("f*x 4.1:", *it, true, true, a[2][3.5], a[2][6], 10.0  * (1.0 + 2.5 * fak), *res);
	checkNext("f*x 4.2:", *it, true, true, a[2][6], a[2.5][1], 10.0  * (1.0 + 5.0 * fak), *res);
	
	checkNext("f*x 4.3:", *it, true, true, a[2.5][1], a[2.5][1.5], 8.0  * (1.0 + 0.0 * fak), *res);
	checkNext("f*x 5:", *it, true, true, a[2.5][1.5], a[2.5][3.5], 8.0  * (1.0 + 0.5 * fak), *res);
	checkNext("f*x 5.1:", *it, true, true, a[2.5][3.5], a[2.5][6], 8.0  * (1.0 + 2.5 * fak), *res);
	checkNext("f*x 5.2:", *it, true, true, a[2.5][6], a[3][1], 8.0  * (1.0 + 5 * fak), *res);
	
	checkNext("f*x 5.3:", *it, true, true, a[3][1], a[3][2], 50.0  * (1.0 + 0.0 * fak), *res);	
	checkNext("f*x 6:", *it, true, true, a[3][2], a[3][3], 50.0  * (1.0 + 1.0 * fak), *res);
	checkNext("f*x 7:", *it, true, true, a[3][3], a[3][3.5], 100.0  * (1.0 + 2.0 * fak), *res);
	checkNext("f*x 7.1:", *it, true, true, a[3][3.5], a[3][6], 100.0  * (1.0 + 2.5 * fak), *res);
	checkNext("f*x 7.2:", *it, true, true, a[3][6], a[3.5][1], 100.0  * (1.0 + 5.0 * fak), *res);
	
	checkNext("f*x 7.3:", *it, true, true, a[3.5][1], a[3.5][3.5], 3.0  * (1.0 + 0.0 * fak), *res);
	checkNext("f*x 8:", *it, true, true, a[3.5][3.5], a[3.5][6], 3.0  * (1.0 + 2.5 * fak), *res);
	checkNext("f*x 8.1:", *it, false, true, a[3.5][6], a[4.5][6], 3.0  * (1.0 + 5.0 * fak), *res);
	
	checkNext("f*x 9:", *it, false, false, a[4.5][6], a[5.5][6], 3.0  * (1.0 + 5.0 * fak), *res);
	delete it;
	delete res;
	
	MultiDimMapping sig(dimSet);
	sig.setValue(a[1][1], 1);
	sig.setValue(a[1][1.5], 1.5);
	sig.setValue(a[1][2], 2);
	sig.setValue(a[1][2.5], 2.5);
	sig.setValue(a[2][1], 3);
	sig.setValue(a[2][1.5], 3.5);
	sig.setValue(a[2][2], 4);
	sig.setValue(a[2][2.5], 4.5);
	
	MultiDimMapping att(dimSet);
	att.setValue(a[1][1.5], 0.0);
	att.setValue(a[1][1.75], 1.0);
	att.setValue(a[1][2.0], 1.0);
	att.setValue(a[1][2.25], 0.0);
	att.setValue(a[2][1.0], 1.0);
	
	res = sig * att;
	
	for(double i = 1; i <= 2.0; i+=1.0) {
		for(double j = 1; j <= 2.5; j+=0.5) {
			if(i == 2) {
				assertEqualSilent("Not attenuated part." + toString(i) + "," + toString(j), j + 2, res->getValue(a[i][j]));
			} else {
				if(j <= 1.6 || j >=2.2)
					assertEqualSilent("To zero attenuated part" + toString(i) + "," + toString(j), 0.0, res->getValue(a[i][j]));
				else
					assertEqualSilent("Not attenuated part" + toString(i) + "," + toString(j), j, res->getValue(a[i][j]));
			}
		}
	}
	delete res;
	
	
}

class TestSimpleConstMapping: public SimpleConstMapping {
public:
	TestSimpleConstMapping(const DimensionSet& dims):
		SimpleConstMapping(dims) {}
	
	TestSimpleConstMapping(const DimensionSet& dims, Argument min, Argument max, Argument interval):
		SimpleConstMapping(dims, min, max, interval) {}
	
	double getValue(const Argument& pos) const {
		return (double)pos.getTime();
	}
	
	ConstMapping* constClone() const { return new TestSimpleConstMapping(*this); }
};

void testPerformance() {
	
	const double chkTime = 1000.0;
	const double factor = 1.0;
	Dimension time("Time");
	Dimension channel("Channel");
	Dimension space("Space");
	
	DimensionSet chTime(Dimension::time);
	chTime.addDimension(channel);
	
	DimensionSet spcChTime(chTime);
	spcChTime.addDimension(space);
	
	Time timer;
	int count = 70000 * factor;
	double el;
	
	Mapping* res;
	{
		TimeMapping f1;
		TimeMapping f2;
		
		std::cout << "--------TimeMapping [" << count << " entries]---------------------------------------------\n";
		std::cout << "Creating f1...\t\t\t\t";
		std::flush(std::cout);
		
		timer.start();
		for(int j = 0; j < count; j++) {
			f1.setValue(Argument(j * 0.1), j * 0.1);
		}
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		std::cout << "Creating f2 ...\t\t\t\t";
		std::flush(std::cout);
		timer.start();
		for(int j = 0; j < count; j++) {
			f2.setValue(Argument(j * 0.1), j * 0.1);
		}
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		std::cout << "Multiplying f1 with f2...\t\t";
		std::flush(std::cout);
		timer.start();
		res = f1 * f2;		
		el = timer.elapsed();
		delete res;
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
	}
	
	
	count = 35000 * factor;
	int hCount = sqrt(count);
	Argument pos(0.0);
	pos.setArgValue(channel, 0.0);	
	
	{
		std::cout << "--------MultiDimFunction [" << count << " entries]----------------------------------------\n";
		std::cout << "Creating f7 ...\t\t\t\t";
		MultiDimMapping f7(chTime);
		MultiDimMapping f8(chTime);
			
		std::flush(std::cout);
		timer.start();
		for(int j = 0; j < count; j++) {
			pos.setTime((j % hCount) * 0.1);
			pos.setArgValue(channel, 0.1 * (int)(j / hCount));
			f7.setValue(pos, j * 0.1);
		}
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		std::cout << "Creating f8 ...\t\t\t\t";
		std::flush(std::cout);
		timer.start();
		for(int j = 0; j < count; j++) {
			pos.setTime((j % hCount) * 0.1);
			pos.setArgValue(channel, 0.1 * (int)(j / hCount));
			f8.setValue(pos, j * 0.1);
		}
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		std::cout << "Multiplying f7 with f8...\t\t";
		std::flush(std::cout);
		timer.start();
		res = f7 * f8;
		el = timer.elapsed();
		delete res;
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
	}
	
	count = 35000 * factor;
	hCount = pow(count, 0.33);
	count = hCount * hCount * hCount;
	
	{
		std::cout << "--------MultiDimFunction 3D [" << count << " entries]--------------------------------------\n";
		std::cout << "Creating f9 and f10...\t\t\t";
		MultiDimMapping f9(spcChTime);
		MultiDimMapping f10(spcChTime);
			
		std::flush(std::cout);
		timer.start();
		int j = 0;
		for(int s = 0; s < hCount; s++) {
			pos.setArgValue(space, s * 0.1);		
			for(int c = 0; c < hCount; c++) {
				pos.setArgValue(channel, c * 0.1);		
				for(int t = 0; t < hCount; t++) {
					pos.setTime(t * 0.1);
					f10.setValue(pos, s * t * c * 0.1);
					f9.setValue(pos, s * t * c * 0.1);
					j++;
				}
			}
		}
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / (count * 2) << "us per entry).\n";
		
		std::cout << "Multiplying f9 with f10...\t\t";
		std::flush(std::cout);
		timer.start();
		res = f9 * f10;
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		
		for(int s = 0; s < hCount; s++) {
			pos.setArgValue(space, s * 0.1);		
			for(int c = 0; c < hCount; c++) {
				pos.setArgValue(channel, c * 0.1);		
				for(int t = 0; t < hCount; t++) {
					pos.setTime(t * 0.1);
					assertClose("3d mult test.",pow(s*t*c*0.1, 2), res->getValue(pos));
				}
			}
		}
		delete res;
		
	}
	
	count = 35000 * factor;
	hCount = sqrt(count);
	count = hCount * hCount;
	
	{
		std::cout << "--------TestSimpleConstMapping [" << count << " entries]----------------------------------\n";
		std::cout << "Creating f13 ...\t\t\t\t";
		Argument from(0.0);
		from.setArgValue(channel, 0.0);
		Argument to(1.0);
		to.setArgValue(channel, 1.0);
		Argument interval(1.0 / hCount);
		interval.setArgValue(channel, 1.0 / hCount);
		
		std::flush(std::cout);
		timer.start();
		TestSimpleConstMapping f13(chTime, from, to, interval);
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		std::cout << "Creating f14...\t\t\t\t";
		std::flush(std::cout);
		timer.start();
		TestSimpleConstMapping f14(chTime, from, to, interval);
		el = timer.elapsed();
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
		std::cout << "Multiplying f13 with f14...\t\t";
		std::flush(std::cout);
		timer.start();
		res = f13 * f14;
		el = timer.elapsed();
		delete res;
		std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry).\n";
	}
}

void checkTestItFromTo(std::string msg, ConstMappingIterator* it, 
					   ConstMapping* f, Argument from, Argument to, Argument step, 
					   std::map<double, std::map<simtime_t, Argument> >& a){
	
	for(double i = from.getArgValue(Dimension("frequency")); 
		i <= to.getArgValue(Dimension("frequency")); 
		i+=step.getArgValue(Dimension("frequency"))) {
		double in = i;
		simtime_t l = to.getTime();
		if(fabs(i - to.getArgValue(Dimension("frequency"))) < 0.0001)
			l -= 0.0001;
		for(simtime_t j = from.getTime(); j <= l; j+=step.getTime()) {
			simtime_t jn = j + step.getTime();
			if(jn > to.getTime()){
				jn = from.getTime();
				in += step.getArgValue(Dimension("frequency"));
			}
			checkIterator(msg, *it, true, true, a[i][j], a[in][jn], j, *f);
			it->next();
		}
	}
}

void testSimpleConstmapping(){
	//displayPassed = true;
	DimensionSet time(Dimension::time);
		
	SimpleConstMapping* f = new TestSimpleConstMapping(time);
	
	for(simtime_t t = -2.2; t <= 2.2; t+=0.2)
		assertEqual("Get value of not fully initialized time-mapping.", t, f->getValue(Argument(t)));
	
	f->initializeArguments(Argument(-2.2), Argument(2.2), Argument(0.2));
	
	for(simtime_t t = -2.2; t <= 2.2; t+=0.2)
		assertEqual("Get value of fully initialized time-mapping.", t, f->getValue(Argument(t)));
	
	ConstMappingIterator* it = f->createConstIterator();
	
	checkIterator("Initial iterator from begin.", *it, true, true, Argument(-2.2), Argument(-2.0), -2.2, *f);
	for(simtime_t t = -2.0; t < 2.19999; t+=0.2){
		checkNext("Next of iterator from begin.", *it, true, true, Argument(t), Argument(t + 0.2), t, *f);
	}
	checkNext("Next of iterator from begin2.", *it, false, true, Argument(2.2), Argument(2.4), 2.2, *f);
	it->iterateTo(Argument(2.4));
	checkIterator("Next of iterator from begin.", *it, false, false, Argument(2.4), Argument(2.6), 2.4, *f);
	it->iterateTo(Argument(2.6));
	checkIterator("Next of iterator from begin.", *it, false, false, Argument(2.6), Argument(2.8), 2.6, *f);
	
	delete it;
	
	it = f->createConstIterator(-2.5);
		
	checkIterator("Initial iterator from pre-begin.", *it, true, false, Argument(-2.5), Argument(-2.2), -2.5, *f);
	for(simtime_t t = -2.2; t < 2.19999999; t+=0.2){
		checkNext("Next of iterator from pre-begin.", *it, true, true, Argument(t), Argument(t + 0.2), t, *f);
	}
	checkNext("Next of iterator from pre-begin.", *it, false, true, Argument(2.2), Argument(2.4), 2.2, *f);
	it->iterateTo(Argument(2.4));
	checkIterator("Next of iterator from pre-begin.", *it, false, false, Argument(2.4), Argument(2.6), 2.4, *f);
	
	delete it;
	
	it = f->createConstIterator(.5);
			
	checkIterator("Initial iterator from post-begin.", *it, true, true, Argument(.5), Argument(.6), .5, *f);
	for(simtime_t t = .6; t < 2.199999; t+=0.2){
		checkNext("Next of iterator from post-begin.", *it, true, true, Argument(t), Argument(t + 0.2), t, *f);
	}
	checkNext("Next of iterator from post-begin.", *it, false, true, Argument(2.2), Argument(2.4), 2.2, *f);
	it->iterateTo(Argument(2.4));
	checkIterator("Next of iterator from post-begin.", *it, false, false, Argument(2.4), Argument(2.6), 2.4, *f);
	
	delete it;
	
	it = f->createConstIterator(2.5);
				
	checkIterator("Initial iterator from post-end.", *it, false, false, Argument(2.5), Argument(2.7), 2.5, *f);
	it->iterateTo(Argument(2.7));
	checkIterator("Next of iterator from post-end.", *it, false, false, Argument(2.7), Argument(2.9), 2.7, *f);
	
	delete it;
	
	delete f;
	
	displayPassed =false;
	
	DimensionSet freqTime(time);
	Dimension freq("frequency");
	freqTime.addDimension(freq);
	
	std::map<double, std::map<simtime_t, Argument> > a;
			
	for(double i = 0.0; i <= 5.5; i+=0.25) {
		for(simtime_t j = 0.0; j <= 5.5; j+=0.25) {
			a[i][j].setTime(j);
			a[i][j].setArgValue(freq, i);
		}
	}
	
	f = new TestSimpleConstMapping(freqTime, a[2][1.5], a[4][4], a[1][0.5]);
	
	for(double i = 0.0; i <= 5.5; i+=0.25) {
		for(simtime_t j = 0.0; j <= 5.5; j+=0.25) {
			assertEqual("Get value of fully initialized freq-time-mapping.", j, f->getValue(a[i][j]));
		}
	}
	
	
	it = f->createConstIterator();
	
	checkTestItFromTo("Next of iterator from begin2.", it, f, a[2][1.5], a[4][4], a[1][0.5], a);
	checkIterator("Next of iterator from begin2.", *it, false, true, a[4][4], a[5][1.5], 4, *f);
	it->iterateTo(a[5][1.5]);
	checkIterator("Next of iterator from begin.", *it, false, false, a[5][1.5], a[5][2.0], 1.5, *f);
	it->iterateTo(a[5][2.0]);
	checkIterator("Next of iterator from begin.", *it, false, false, a[5][2.0], a[5][2.5], 2.0, *f);
	delete it;
	
	it = f->createConstIterator(a[1][1]);
	
	checkIterator("Initial iterator from pre freq, pre time begin.", *it, true, false, a[1][1], a[2][1.5], 1, *f);
	it->next();
	checkTestItFromTo("Next of iterator from pre freq, pre time begin.", it, f, a[2][1.5], a[4][4], a[1][0.5], a);
	checkIterator("Next of iterator from pre freq, pre time begin.", *it, false, true, a[4][4], a[5][1.5], 4, *f);
	it->iterateTo(a[5][1.5]);
	checkIterator("Next of iterator from pre freq, pre time begin.", *it, false, false, a[5][1.5], a[5][2.0], 1.5, *f);
	it->iterateTo(a[5][2.0]);
	checkIterator("Next of iterator from pre freq, pre time begin.", *it, false, false, a[5][2.0], a[5][2.5], 2.0, *f);
	delete it;
	
	it = f->createConstIterator(a[1][2]);
		
	checkIterator("Initial iterator from pre freq, post time begin.", *it, true, false, a[1][2], a[2][1.5], 2, *f);
	it->next();
	checkTestItFromTo("Next of iterator from pre freq, post time begin.", it, f, a[2][1.5], a[4][4], a[1][0.5], a);
	checkIterator("Next of iterator from pre freq, post time begin.", *it, false, true, a[4][4], a[5][1.5], 4, *f);
	it->iterateTo(a[5][1.5]);
	checkIterator("Next of iterator from pre freq, post time begin.", *it, false, false, a[5][1.5], a[5][2.0], 1.5, *f);
	it->iterateTo(a[5][2.0]);
	checkIterator("Next of iterator from pre freq, post time begin.", *it, false, false, a[5][2.0], a[5][2.5], 2.0, *f);
	delete it;
	
	it = f->createConstIterator(a[3][0.5]);
			
	checkIterator("Initial iterator from post freq, pre time begin.", *it, true, true, a[3][0.5], a[3][1.5], 0.5, *f);
	it->next();
	checkTestItFromTo("Next of iterator from post freq, pre time begin.", it, f, a[3][1.5], a[4][4], a[1][0.5], a);
	checkIterator("Next of iterator from post freq, pre time begin.", *it, false, true, a[4][4], a[5][1.5], 4, *f);
	it->iterateTo(a[5][1.5]);
	checkIterator("Next of iterator from post freq, pre time begin.", *it, false, false, a[5][1.5], a[5][2.0], 1.5, *f);
	it->iterateTo(a[5][2.0]);
	checkIterator("Next of iterator from post freq, pre time begin.", *it, false, false, a[5][2.0], a[5][2.5], 2.0, *f);
	delete it;
	
	it = f->createConstIterator(a[3][5]);
	
	checkIterator("Initial iterator from pre freq, post time end.", *it, true, true, a[3][5], a[4][1.5], 5, *f);
	it->next();
	checkTestItFromTo("Next of iterator from pre freq, post time end.", it, f, a[4][1.5], a[4][4], a[1][0.5], a);
	checkIterator("Next of iterator from pre freq, post time end.", *it, false, true, a[4][4], a[5][1.5], 4, *f);
	it->iterateTo(a[5][1.5]);
	checkIterator("Next of iterator from pre freq, post time end.", *it, false, false, a[5][1.5], a[5][2.0], 1.5, *f);
	it->iterateTo(a[5][2.0]);
	checkIterator("Next of iterator from pre freq, post time end.", *it, false, false, a[5][2.0], a[5][2.5], 2.0, *f);
	delete it;
	
	it = f->createConstIterator(a[5][2]);
		
	checkIterator("Initial iterator from post freq, pre time end.", *it, false, false, a[5][2], a[4][1.5], 2, *f);
	it->iterateTo(a[5][2.5]);
	checkIterator("Next of iterator from post freq, pre time end.", *it, false, false, a[5][2.5], a[5][2.5], 2.5, *f);
	delete it;
	
	it = f->createConstIterator(a[5][4.5]);
			
	checkIterator("Initial iterator from post end.", *it, false, false, a[5][4.5], a[4][1.5], 4.5, *f);
	it->iterateTo(a[5][5]);
	checkIterator("Next of iterator from post end.", *it, false, false, a[5][5], a[5][2.5], 5, *f);
	delete it;
	
	delete f;	
	
	displayPassed = false;
}

//TODO: run test, possibly check setting of position at jumpTo and co


int main() {
	displayPassed = false;
	testDimension();
	testArg();
	//testDoubleCompareLess();
	
    testSimpleFunction<TimeMapping>();
    std::cout << "--------TimeMapping tests done.---------------------------------------------------\n";
    
    
    testMultiFunction();
    std::cout << "--------Multifunction tests done.-------------------------------------------------\n";
    
    testSimpleConstmapping();
    std::cout << "--------SimpleConstMapping tests done.--------------------------------------------\n";
    
    std::cout << "========Performance tests=========================================================\n";
    testPerformance();
    
    
}
