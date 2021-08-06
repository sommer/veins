//
// Copyright (C) 2021 Christoph Sommer <sommer@cms-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/veins.h"

namespace {
#if OMNETPP_BUILDNUM >= 1020
static cNEDValue opp_eval(const char* txt, cExpression::Context* context)
#else
static cNEDValue opp_eval(const char* txt, cComponent* context)
#endif
{
    try {
        cDynamicExpression expr;
#if OMNETPP_BUILDNUM >= 1525
        expr.parseNedExpr(txt);
#elif OMNETPP_BUILDNUM >= 1500
        expr.parseNedExpr(txt, true, true);
#else
        expr.parse(txt);
#endif
        return expr.evaluate(context);
    }
    catch (std::exception& e) {
        throw cRuntimeError("Error evaluating expression \"%s\": %s", txt, e.what());
    }
}
} // namespace

namespace veins {

#if OMNETPP_VERSION >= 0x600
cNEDValue nedf_veins_eval_by_version(cExpression::Context* context, cNEDValue argv[], int argc)
#else
cNEDValue nedf_veins_eval_by_version(cComponent* component, cNEDValue argv[], int argc)
#endif
{
    if (argc < 2) {
        throw cRuntimeError("veins_eval_by_version(): at least 2 arguments expected");
    }
    if (argv[0].getType() != cNEDValue::INT) {
        throw cRuntimeError("veins_eval_by_version(): int argument expected for argument %d", 0);
    }
    if (argv[1].getType() != cNEDValue::STRING) {
        throw cRuntimeError("veins_eval_by_version(): string argument expected for argument %d", 1);
    }

    int ver = (int) argv[0];
    int idx = 0;

    while (idx + 4 <= argc) {
        if (argv[idx + 2].getType() != cNEDValue::INT) {
            throw cRuntimeError("veins_eval_by_version(): int argument expected for argument %d", idx + 2);
        }
        if (argv[idx + 3].getType() != cNEDValue::STRING) {
            throw cRuntimeError("veins_eval_by_version(): string argument expected for argument %d", idx + 3);
        }
        int v = (int) argv[idx + 2];
        if (ver >= v) {
            idx = idx + 2;
        }
        else {
            break;
        }
    }

#if OMNETPP_VERSION >= 0x600
    return opp_eval(argv[idx + 1].stringValue(), context);
#elif OMNETPP_BUILDNUM >= 1020
    cExpression::Context context(component);
    return opp_eval(argv[idx + 1].stringValue(), &context);
#else
    return opp_eval(argv[idx + 1].stringValue(), component);
#endif
}

Define_NED_Function2(nedf_veins_eval_by_version,
    "any veins_eval_by_version(int version, string defaultExpression, int minVersion1, string expression1, ...)",
    "veins",
    "Evaluates to expression1 if version >= minVersion1, otherwise evaluates to defaultExpression");

#if OMNETPP_BUILDNUM > 1525
cNEDValue nedf_veins_omnetpp_buildnum(cExpression::Context* context, cNEDValue argv[], int argc)
#else
cNEDValue nedf_veins_omnetpp_buildnum(cComponent* component, cNEDValue argv[], int argc)
#endif
{
#if OMNETPP_BUILDNUM > 1525
    return (intval_t) OMNETPP_BUILDNUM;
#else
    return (int) OMNETPP_BUILDNUM;
#endif
}

Define_NED_Function2(nedf_veins_omnetpp_buildnum,
    "int veins_omnetpp_buildnum()",
    "veins",
    "Returns value of C++ OMNETPP_BUILDNUM macro");

} // namespace veins
