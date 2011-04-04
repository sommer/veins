//
// InlineXmlDoc - provides inline xml functions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "utils/InlineXmlDoc.h"

Define_NED_Function2(nedf_emptyxmldoc, "xml emptyxmldoc(string s)", "strings", "returns an empty XML document <s></s>.");

static cDynamicExpression::Value nedf_emptyxmldoc(cComponent *context, cDynamicExpression::Value argv[], int argc) {
	if (argc != 1) throw cRuntimeError("emptyxmldoc(): takes exactly 1 argument");
	std::string s = argv[0].s;

	cXMLElement* root = new cXMLElement(s.c_str(), "inline", 0);

	return cDynamicExpression::Value(root);
}

