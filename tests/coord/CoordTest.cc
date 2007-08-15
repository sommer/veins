/***************************************************************************
                          CoordTest.cc  -  description
                             -------------------
    begin                : Wed Aug 15 2007
    copyright            : (C) 2007 by wessel
    email                : wessel@tkn.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <Coord.h>
#include <FWMath.h>
#include <iostream>

using namespace std;

const double X = 2.4;
const double Y = 1.2;
const double Z = 4.8;
const double X2 = 1.6;
const double Y2 = 1.2;
const double Z2 = 0.2;
const double FAKT = 2.1;
const double TOLERANCE = 0.0000001;

const double SUM_X = X + X2;
const double DIFF_X = X - X2;
const double SUM_Y = Y + Y2;
const double DIFF_Y = Y - Y2;
const double SUM_Z = Z + Z2;
const double DIFF_Z = Z - Z2;

const double FAK_X = X * FAKT;
const double FAK_Y = Y * FAKT;
const double FAK_Z = Z * FAKT;

const double XYZ_SQUARE_LENGTH = X * X + Y * Y + Z * Z;
const double XYZ_LENGTH = sqrt(XYZ_SQUARE_LENGTH);

const double XY_SQUARE_LENGTH = X * X + Y * Y;
const double XY_LENGTH = sqrt(XY_SQUARE_LENGTH);

const double PG_X = 5.0;
const double PG_Y = 5.0;
const double PG_Z = 5.0;

const double SQUARE_DISTANCE_3D = DIFF_X * DIFF_X + DIFF_Y * DIFF_Y + DIFF_Z * DIFF_Z;
const double DISTANCE_3D = sqrt(SQUARE_DISTANCE_3D);

const double SQUARE_DISTANCE_2D = DIFF_X * DIFF_X + DIFF_Y * DIFF_Y;
const double DISTANCE_2D = sqrt(SQUARE_DISTANCE_2D);



void assertTrue(string msg, bool value) {
    if (!value) {
        cout << msg << " -- failed" << endl;
        exit(1);
    }
}

void assertFalse(string msg, bool value) { assertTrue(msg, !value); }

void assertEqual(string msg, double target, double actual) {
    if (!FWMath::close(target, actual)) {
        cout << msg << " -- failed: value was "
             << actual << " instead of " << target << endl;
        exit(1);
    }
}

/**
 * unit test for constructors of class Coord
 *
 * - test default constructor
 * - test Coord(bool use2D)
 * - test Coord(x, y)
 * - test Coord(x, y, z)
 * - test Coord(Coord* other)
 * - test Coord(Coord& other)
 *
 * assumes correctness of following methods:
 * - getX(), getY(), getZ()
 * - operator=()
 * - is2D(), is3D()
 */
void testConstructors() {
    
    // test default constructor
    Coord testCoord;
    assertTrue("Default constructor creates 3D-Coord.", testCoord.is3D());
    assertFalse("Default constructor creates no 2D-Coord.", testCoord.is2D());
    assertEqual("x-value of default coordinate.", 0.0, testCoord.getX());
    assertEqual("y-value of default coordinate.", 0.0, testCoord.getY());
    assertEqual("z-value of default coordinate.", 0.0, testCoord.getZ());

    //test Coord(bool use2D)
    testCoord = Coord(false);
    assertTrue("Coord(false) constructor creates 3D-Coord.", testCoord.is3D());
    assertFalse("Coord(false) constructor creates no 2D-Coord.", testCoord.is2D());
    assertEqual("x-value of Coord(false).", 0.0, testCoord.getX());
    assertEqual("y-value of Coord(false).", 0.0, testCoord.getY());
    assertEqual("z-value of Coord(false).", 0.0, testCoord.getZ());

    testCoord = Coord(true);
    assertFalse("Coord(true) constructor creates no 3D-Coord.", testCoord.is3D());
    assertTrue("Coord(true) constructor creates 2D-Coord.", testCoord.is2D());
    assertEqual("x-value of Coord(false).", 0.0, testCoord.getX());
    assertEqual("y-value of Coord(false).", 0.0, testCoord.getY());
    assertEqual("z-value of Coord(false).", Coord::UNDEFINED, testCoord.getZ());
    
    //test Coord(x, y)
    
    testCoord = Coord(X, Y);
    assertFalse("Coord(x, y) constructor creates no 3D-Coord.", testCoord.is3D());
    assertTrue("Coord(x, y) constructor creates 2D-Coord.", testCoord.is2D());
    assertEqual("x-value of Coord(x, y).", X, testCoord.getX());
    assertEqual("y-value of Coord(x, y).", Y, testCoord.getY());
    assertEqual("z-value of Coord(x, y).", Coord::UNDEFINED, testCoord.getZ());
    
    //test Coord(x, y, z)
    
    testCoord = Coord(X, Y, Z);
    assertTrue("Coord(x, y, z) constructor creates 3D-Coord.", testCoord.is3D());
    assertFalse("Coord(x, y, z) constructor creates no 2D-Coord.", testCoord.is2D());
    assertEqual("x-value of Coord(x, y, z).", X, testCoord.getX());
    assertEqual("y-value of Coord(x, y, z).", Y, testCoord.getY());
    assertEqual("z-value of Coord(x, y, z).", Z, testCoord.getZ());
    
    //test Coord(Coord* other)
    //test Coord(Coord& other)
    Coord testCoord2(&testCoord);
    assertTrue("(3D)Coord(Coord* other) constructor same dimension as other.",
                testCoord.is3D() == testCoord2.is3D());
    assertEqual("x-value of (3D)Coord(Coord* other).", testCoord.getX(), testCoord2.getX());
    assertEqual("y-value of (3D)Coord(Coord* other).", testCoord.getY(), testCoord2.getY());
    assertEqual("z-value of (3D)Coord(Coord* other).", testCoord.getZ(), testCoord2.getZ());

    testCoord2 = Coord(testCoord);
    assertTrue("Coord(Coord& other) constructor same dimension as other.",
                testCoord.is3D() == testCoord2.is3D());
    assertEqual("x-value of (3D)Coord(Coord& other).", testCoord.getX(), testCoord2.getX());
    assertEqual("y-value of (3D)Coord(Coord& other).", testCoord.getY(), testCoord2.getY());
    assertEqual("z-value of (3D)Coord(Coord& other).", testCoord.getZ(), testCoord2.getZ());

    testCoord = Coord(X, Y);
    testCoord2 = Coord(&testCoord);
    assertTrue("(2D)Coord(Coord* other) constructor same dimension as other.",
                testCoord.is3D() == testCoord2.is3D());
    assertEqual("x-value of (2D)Coord(Coord* other).", testCoord.getX(), testCoord2.getX());
    assertEqual("y-value of (2D)Coord(Coord* other).", testCoord.getY(), testCoord2.getY());
    assertEqual("z-value of (2D)Coord(Coord* other).", testCoord.getZ(), testCoord2.getZ());

    
    testCoord2 = Coord(testCoord);
    assertTrue("(2D)Coord(Coord& other) constructor same dimension as other.",
                testCoord.is3D() == testCoord2.is3D());
    assertEqual("x-value of (2D)Coord(Coord& other).", testCoord.getX(), testCoord2.getX());
    assertEqual("y-value of (2D)Coord(Coord& other).", testCoord.getY(), testCoord2.getY());
    assertEqual("z-value of (2D)Coord(Coord& other).", testCoord.getZ(), testCoord2.getZ());

    cout << "Constructor tests successful." << endl;
    
}

/**
 * unit test for operator methods of class Coord
 *
 * - test plus, minus, scalar multiplication
 * - test in 2D/3D
 * - preserve UNDEFINED of z at 2D
 */
void testOperators() {

	// plus (3D)
	Coord a(X, Y, Z);
	Coord b(X2, Y2, Z2);

	Coord erg = a + b;
	assertTrue("3D: Result of a + b is 3D", erg.is3D());
	assertEqual("3D: x-value of a + b", SUM_X, erg.getX());
	assertEqual("3D: y-value of a + b", SUM_Y, erg.getY());
	assertEqual("3D: z-value of a + b", SUM_Z, erg.getZ());

	erg = a - b;
	assertTrue("3D: Result of a - b is 3D", erg.is3D());
	assertEqual("3D: x-value of a - b", DIFF_X, erg.getX());
	assertEqual("3D: y-value of a - b", DIFF_Y, erg.getY());
	assertEqual("3D: z-value of a - b", DIFF_Z, erg.getZ());

	erg = a * FAKT;
	assertTrue("3D: Result of a * faktor is 3D", erg.is3D());
	assertEqual("3D: x-value of a * faktor", FAK_X, erg.getX());
	assertEqual("3D: y-value of a * faktor", FAK_Y, erg.getY());
	assertEqual("3D: z-value of a * faktor", FAK_Z, erg.getZ());

    erg = a / (1.0 / FAKT);
	assertTrue("3D: Result of a / (1 / faktor) is 3D", erg.is3D());
	assertEqual("3D: x-value of a / (1 / faktor)", FAK_X, erg.getX());
	assertEqual("3D: y-value of a / (1 / faktor)", FAK_Y, erg.getY());
	assertEqual("3D: z-value of a / (1 / faktor)", FAK_Z, erg.getZ());

    //2D

    a = Coord(X, Y);
	b = Coord(X2, Y2);

    erg = a + b;
	assertTrue("2D: Result of a + b is 2D", erg.is2D());
	assertEqual("2D: x-value of a + b", SUM_X, erg.getX());
	assertEqual("2D: y-value of a + b", SUM_Y, erg.getY());
	assertEqual("2D: z-value UNDEFINED of a + b", Coord::UNDEFINED, erg.getZ());

	erg = a - b;
	assertTrue("2D: Result of a - b is 2D", erg.is2D());
	assertEqual("2D: x-value of a - b", DIFF_X, erg.getX());
	assertEqual("2D: y-value of a - b", DIFF_Y, erg.getY());
	assertEqual("2D: z-value UNDEFINED of a - b", Coord::UNDEFINED, erg.getZ());

	erg = a * FAKT;
	assertTrue("2D: Result of a * faktor is 2D", erg.is2D());
	assertEqual("2D: x-value of a * faktor", FAK_X, erg.getX());
	assertEqual("2D: y-value of a * faktor", FAK_Y, erg.getY());
	assertEqual("2D: z-value UNDEFINED of a * faktor", Coord::UNDEFINED, erg.getZ());

    erg = a / (1.0 / FAKT);
	assertTrue("2D: Result of a / (1 / faktor) is 2D", erg.is2D());
	assertEqual("2D: x-value of a / (1 / faktor)", FAK_X, erg.getX());
	assertEqual("2D: y-value of a / (1 / faktor)", FAK_Y, erg.getY());
	assertEqual("2D: z-value UNDEFINED of a / (1 / faktor)", Coord::UNDEFINED, erg.getZ());

	cout << "Operator tests successful." << endl;
}

/**
 * Unit test for compare operators of class Coord
 *
 * - test "==" and "!="
 * - test for floating point failure tolerance
 * - test in 2D/3D
 */
void testCompareOperators() {

    // 3D
    Coord a(X, Y, Z);
    Coord b(X, Y, Z);
    Coord aTol(X + TOLERANCE, Y + TOLERANCE, Z + TOLERANCE);

    assertTrue("3D: a == b", a == b);
    assertFalse("3D: a != b", a != b);
    assertTrue("3D: a == a + tolerance", a == aTol);
    assertFalse("3D: a != a + tolerance", a != aTol);

    // 2D
    a = Coord(X, Y);
    b = Coord(X, Y);
    aTol = Coord(X + TOLERANCE, Y + TOLERANCE);

    assertTrue("2D: a == b", a == b);
    assertFalse("2D: a != b", a != b);
    assertTrue("2D: a == a + tolerance", a == aTol);
    assertFalse("2D: a != a + tolerance", a != aTol);

    cout << "Compare operator test successful." << endl;
    
}

/**
 * Unit test for length methods of class Coord
 *
 * - test length() and sqrLength()
 * - test 2D / 3D
 */
void testLength() {
    Coord a;

    //3D
    assertEqual("3D: length of origin-vector.", 0.0, a.length());
    assertEqual("3D: square length of origin-vector.", 0.0, a.squareLength());

    a = Coord(X, Y, Z);
    assertEqual("3D: length of (x, y, z)-vector.", XYZ_LENGTH, a.length());
    assertEqual("3D: square length of (x, y, z)-vector.", XYZ_SQUARE_LENGTH, a.squareLength());

    //2D
    a = Coord(true);
    assertEqual("2D: length of origin-vector.", 0.0, a.length());
    assertEqual("2D: square length of origin-vector.", 0.0, a.squareLength());

    a = Coord(X, Y);
    assertEqual("2D: length of (x, y, z)-vector.", XY_LENGTH, a.length());
    assertEqual("2D: square length of (x, y, z)-vector.", XY_SQUARE_LENGTH, a.squareLength());

    cout << "Length methods test successful." << endl;
}

/**
 * Unit test for distance methods of class Coord
 *
 * - test distance(), sqrdist() and sqrTorusDist()
 * - test 2D/3D
 */
void testDistance() {

    //3D
    Coord a(X, Y, Z);
    Coord b(X2, Y2, Z2);

    assertEqual("3D: square distance a<->b", SQUARE_DISTANCE_3D, a.sqrdist(b));
    assertEqual("3D: distance a<->b", DISTANCE_3D, a.distance(b));

    //TODO: test torus distance

    //2D
    a = Coord(X, Y);
    b = Coord(X2, Y2);

    assertEqual("2D: square distance a<->b", SQUARE_DISTANCE_2D, a.sqrdist(b));
    assertEqual("2D: distance a<->b", DISTANCE_2D, a.distance(b));

    //TODO: test torus distance
    
    cout << "Distance methods test successful." << endl;
}

int main() {
    testConstructors();
    testOperators();
    testCompareOperators();
    testLength();
    testDistance();
    
}

