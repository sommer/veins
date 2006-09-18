/** 
   mixim_generator - Generates config files for use with MiXiM

   Copyright (C) 2006 Otto Visser

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
*/

#include "mixim_generator.h"

/* Option flags and variables.  These are initialized in parse_opt.  */

static char *desired_directory;		/* --directory=DIR */
static int debugLevel = 0;		/* quiet, silent, verbose */

#ifndef __WIN32__
static error_t parse_opt(int key, char *arg, struct argp_state *state);
static void show_version(FILE * stream, struct argp_state *state);

static struct argp_option options[] = {
	{"quiet", 'q', NULL, 0, N_("Inhibit usual output"), 0},
	{"silent", 0, NULL, OPTION_ALIAS, NULL, 0},
	{"verbose", 'v', NULL, 0, N_("Print more information"), 0},
	{"directory", 'd', N_("DIR"), 0, N_("Use directory DIR"), 0},
	{NULL, 0, NULL, 0, NULL, 0}
};

/* The argp functions examine these global variables.  */
const char *argp_program_bug_address = "<o.w.visser@tudelft.nl>";
void (*argp_program_version_hook) (FILE *, struct argp_state *) = show_version;

static struct argp argp = {
	options, parse_opt, N_("[FILE...]"),
	N_("Generates config files for use with MiXiM"),
	NULL, NULL, NULL
};
#endif

int main(int argc, char **argv) {
	unsigned nodeCount;
	int res;
//	int choice;
	char *input;
	char* path;

	input = (char*)malloc(BUFSIZE);
	path = (char*)malloc(1024);
	if (input == NULL || path == NULL) {
		printf("Out of memory!\n");
		exit(EXIT_FAILURE);
	}

	textdomain(PACKAGE);
#ifndef __WIN32__
	if (argp_parse(&argp, argc, argv, 0, NULL, NULL) != 0) {
		perror("Error parsing parameters");
		exit(EXIT_FAILURE);
	}
	res = snprintf(path, 1024, "%s/omnetpp.ini", desired_directory);
#else
	res = snprintf(path, 1024, "..\\omnetpp.ini");
#endif	
	if (res < 0 || res > 1024) {
		printf("An error occured; check the specified directory");
		exit(EXIT_FAILURE);
	}
	init(path);
	/* general part */
	addConfig("[General]");
	addConfigInt("ini-warnings", 1);
	addConfigInt("debug-on-errors", 1);
	addConfigString("network", "net");

	do {
		printf("Maximum simulated time? <value>(smhd) ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!checkTimeValue(input));
	writeConfig("sim-time-limit", input, false);

	do {
		printf("Maximum computer time? <value>(smhd) ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!(empty(input) || checkTimeValue(input)));
	if (!empty(input))
		writeConfig("cpu-time-limit", input, false);

	writeConfig("num-rngs", "6", false);
	writeConfig("rng-class", "cLCG32", false);
	writeConfig("seed-0-lcg32 ", "1328964554", false);
	writeConfig("seed-1-lcg32 ", "1328964554", false);
	writeConfig("seed-2-lcg32 ", "1328964554", false);
	writeConfig("seed-3-lcg32 ", "1328964554", false);
	writeConfig("seed-4-lcg32 ", "1328964554", false);
	writeConfig("seed-5-lcg32 ", "1328964554", false);

	/* Cmdenv part */
	addConfig("\n");
	addConfig("[Cmdenv]");
	addConfigInt("event-banners", 0);
	
	/* parameters part */
	addConfig("\n");
	addConfig("[Parameters]");

	do {
		printf("How many nodes? ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!checkNumber(input));
	nodeCount = atoi(input);
	addConfigInt("net.nodeCount", nodeCount);

	// node distribution
/*	do {
		printf("How to distribute the nodes?\n \
			1) Random\n \
			2) Regular grid\n \
			3) Ask when starting program\n");
		choice = fgetc(stdin);
	} while (choice < 1 || choice > 3);
	switch (choice) {
		case 1:
			// random
	}
			
*/	

	do {
		printf("What node type? (TNOde|mica2|mica2dot) ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!checkString(input));	
	// TODO: check values for correctness
	addConfigString("net.nodeType", input);
	// TODO make this configurable? Dependant on chosen radiopower?
	addConfigInt("net.radioRange", 25);

	// TODO: make dependant on node type?
	do {
		printf("What radio type? (CC1000|RFM1001) ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!checkString(input));	
	// TODO: check values for correctness
	addConfigString("net.radioType", input);

	// TODO not configurable yet (due to lack of choice ;) )
	addConfigString("net.ultrasoundType", "SimpleUS");
	addConfigString("net.radioPropagationType", "SimpleRadioPropagationModel");
	addConfigString("net.usPropagationType", "SimpleUSPropagationModel");
	addConfigString("net.macType", "TMac");

	do {
		printf("What routing layer? (NoRouting|...) ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!checkString(input));	
	addConfigString("net.routingType", input);
	
	do {
		printf("What is the name of the application? ");
		fgets(input, BUFSIZE, stdin);
		stripNewline(input);
	} while (!checkString(input));	
	// TODO: check values for correctness
	addConfigString("net.applicationName", input);

	// TODO: not configurable yet
	addConfigInt("net.lpl", 0);
	addConfigInt("net.lpl_on", 0);
	addConfigInt("net.lpl_off", 0);
	addConfigInt("net.siftNodes", 512);
	
	free(input);
	free(path);
	exit(EXIT_SUCCESS);
}

#ifndef __WIN32__
/* Parse a single option.  */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	switch (key) {
		case ARGP_KEY_INIT:
			/* Set up default values.  */
			desired_directory = ".";
			debugLevel = 0;
			break;

		case 'q':				/* --quiet, --silent */
			debugLevel--;
			break;
		case 'v':				/* --verbose */
			debugLevel++;
			break;
		case 'd':	/* --directory */
			desired_directory = strdup(arg);
			printf("Chose dir: %s\n", arg);
			break;

		case ARGP_KEY_ARG:		/* [FILE]... */
			/* TODO: Do something with ARG, or remove this case and make
			   main give argp_parse a non-NULL fifth argument.  */
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Show the version number and copyright information.  */
static void show_version(FILE * stream, struct argp_state *state) {
	(void) state;
	/* Print in small parts whose localizations can hopefully be copied
	   from other programs.  */
	fputs(PACKAGE " " VERSION "\n", stream);
	fprintf(stream, _("Written by %s.\n\n"), "Otto Visser");
	fprintf(stream, _("Copyright (C) %s %s\n"), "2006", "Otto Visser");
	fputs(_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License.  This program has absolutely no warranty.\n"), stream);
}
#endif
