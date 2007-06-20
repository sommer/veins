/***************************************************************************
 * file:        TestLocAppl.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: application layer: test class for the application layer
 ***************************************************************************/

/* ************************************************************************
 * Peterpaul Klein Haneveld:
 **************************************************************************
 * This file is essentially a copy of TestApplLayer with just the following
 * replacements:
 *
 * TestApplLayer        -> TestLocAppl
 * BaseApplLayer        -> BaseLocAppl
 **************************************************************************/

#include "TestLocAppl.h"

Define_Module_Like(TestLocAppl, BaseLocAppl);

void TestLocAppl::initialize(int stage)
{
	BaseLocAppl::initialize(stage);
	
	if (stage == 1) {
		if (loc != NULL) {
			fprintf(stdout, 
				"I Guess that i've found BaseLocalization for %d.\n",
				myApplAddr());
		} else {
			fprintf (stdout, 
				 "...\n");
		}
	}
}
