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

FILE* omnetFile;
char* buffer;

void init(const char* path) {
	buffer = (char*)malloc(BUFSIZE);
	if (buffer == NULL) {
		printf("Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	omnetFile = fopen(path, "w+b");
	if (omnetFile == NULL) {
		perror("Error opening config file for writing");
		exit(EXIT_FAILURE);
	}
	//printf("Will put file in %s\n", path);
}

void finish() {
	free(buffer);
	if (fclose(omnetFile) != 0) {
		perror("Error closing config file");
	}
}

void writeConfig(char *option, const char *value, bool quoted) {
	int res;
	
	if (value == NULL || strlen(value) == 0)
		sprintf(buffer, "%s\n%c", option, 0x00);
	else {
		if (quoted) {
			sprintf(buffer, "%s=\"%s\"\n%c", option, value, 0x00);
		} else {
			sprintf(buffer, "%s=%s\n%c", option, value, 0x00);
		}
	}

	res = fwrite(buffer, 1, strlen(buffer), omnetFile);
	if (res == 0) {
		printf("An error occured while writing the config file\n");
	}
}

void addConfigString(char *option, const char *value) {
	writeConfig(option, value, true);
}

void addConfig(char *option) {
	writeConfig(option, NULL, false);
}

void addConfigInt(char *option, int value) {
	char temp[64];

	sprintf(temp, "%d", value);
	writeConfig(option, temp, false);
}

void addConfigDouble(char *option, double value) {
	char temp[64];

	sprintf(temp, "%f", value);
	writeConfig(option, temp, false);
}

void stripNewline(char *line) {
	size_t length;
	
	length = strlen(line);
	if (length > 0 && line[length - 1] == '\n')
		line[length - 1] = 0;
}

bool checkNumber(char *line) {
	long value;
	char *endptr;
	size_t length;
	
	length = strlen(line);
	errno = 0;
	
	while (isspace(*line)) line++;
	if (!isdigit(*line) || (*line == '-' && !isdigit(*(line+1)) )) {
		printf("No value supplied\n");
		return false;
	}
	
	value = strtol(line, &endptr, 10);
	
	if (errno != 0) {
		printf("Value (%ld) out of range\n", value);
		return false;
	}

	while (isspace(*endptr)) endptr++;
	if (*endptr != 0) {
		printf("Garbage after value\n");
		return false;
	}
	return true;
}

bool checkTimeValue(char *line) {
	long value;
	char *endptr;
	size_t length;
	
	length = strlen(line);
	errno = 0;
	
	while (isspace(*line)) line++;
	if (!isdigit(*line) && *line != '-') {
		printf("No value supplied\n");
		return false;
	}
	
	value = strtol(line, &endptr, 10);
	
	if (errno != 0 || value <= 0) {
		printf("Value (%ld) out of range\n", value);
		return false;
	}
	
	while (isspace(*endptr)) endptr++;
	
	if (*endptr == 0) {
		printf("No time base supplied\n");
		return false;
	}
	
	if (strchr("smhd", *endptr) == NULL) {
		printf("Invalid time base supplied\n");
		return false;
	}
	
	endptr++;
	while (isspace(*endptr)) endptr++;

	if (*endptr != 0) {
		printf("Garbage after time value\n");
		return false;
	}
	return true;
}

bool empty(char *line) {
	while (isspace(*line)) line++;
	return *line == 0;
}

bool checkString(char *line) {
	if (strchr(line, '"') != NULL) {
		printf("Quotes are not allowed in strings\n");
		return false;
	}
	if (empty(line))
		return false;
	return true;
}

void writeNode(unsigned nodeId, double x, double y, double z) {
	static int nodeIndex = 0;
	char paramBuffer[1024];
	
	snprintf(paramBuffer, 1024, "net.nodes[%d].node.node_id", nodeIndex);
	addConfigInt(paramBuffer, nodeId);
	
	snprintf(paramBuffer, 1024, "net.nodes[%d].node.pos_x", nodeIndex);
	addConfigDouble(paramBuffer, x);
	snprintf(paramBuffer, 1024, "net.nodes[%d].node.pos_y", nodeIndex);
	addConfigDouble(paramBuffer, y);
	snprintf(paramBuffer, 1024, "net.nodes[%d].node.pos_z", nodeIndex);
	addConfigDouble(paramBuffer, z);
	nodeIndex++;
}

