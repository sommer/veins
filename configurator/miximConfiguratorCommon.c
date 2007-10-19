/** 
   mixim_generator - Generates config files for use with MiXiM

   Copyright (C) 2006-2007 Otto Visser

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

#include "miximConfiguratorCommon.h"

FILE* omnetFile;
char* buffer;
char* sourcePath;

void init(const char* configPath, const char* _sourcePath) {
	sourcePath = malloc(1024);
	buffer = (char*)malloc(BUFSIZE);

	if (buffer == NULL || sourcePath == NULL) {
		printf("Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	omnetFile = fopen(configPath, "w+b");
	if (omnetFile == NULL) {
		perror("Error opening config file for writing");
		exit(EXIT_FAILURE);
	}
	//printf("Will put file in %s\n", path);
	
	strncpy(sourcePath, _sourcePath, 1023);
	
	// CmdEnv part
	addConfig("[Cmdenv]");
	addConfigInt("event-banners", 0);

	//the general(/compulsory) part
	addConfig("\n");
	addConfig("[General]");
	addConfigInt("ini-warnings", 1);
	addConfigInt("debug-on-errors", 1);
	addConfigString("network", "net");
	
	writeConfig("num-rngs", "6", false);
	writeConfig("rng-class", "cLCG32", false);
	writeConfig("seed-0-lcg32 ", "1328964554", false);
	writeConfig("seed-1-lcg32 ", "1328964554", false);
	writeConfig("seed-2-lcg32 ", "1328964554", false);
	writeConfig("seed-3-lcg32 ", "1328964554", false);
	writeConfig("seed-4-lcg32 ", "1328964554", false);
	writeConfig("seed-5-lcg32 ", "1328964554", false);
}

void startParameterSection() {
	/* parameters part */
	addConfig("\n");
	addConfig("[Parameters]");
}

void finish() {
	free(buffer);
	if (fclose(omnetFile) != 0)
		perror("Error closing config file");
}

int nedCheck(const struct dirent* entry) {
	if (entry->d_type == DT_REG) {
		char end[4];
		unsigned length = strlen(entry->d_name);
		if (length < 4)
			return 0;
		memcpy(&end, &(entry->d_name[length - 4]), 4);
		if (strncmp(end, ".ned", 4) == 0) {
			printf("found match: %s\n", entry->d_name);
			return 1; // match found
		}
	}
	return 0;
}

void writeConfig(const char *option, const char *value, bool quoted) {
	if (value == NULL || strlen(value) == 0) {
		sprintf(buffer, "%s\n%c", option, 0x00);
	} else {
		if (quoted) {
			sprintf(buffer, "%s=\"%s\"\n%c", option, value, 0x00);
		} else {
			sprintf(buffer, "%s=%s\n%c", option, value, 0x00);
		}
	}

	if (fwrite(buffer, 1, strlen(buffer), omnetFile) == 0)
		printf("An error occured while writing the config file\n");
}

void addConfigString(const char *option, const char *value) {
	writeConfig(option, value, true);
}

void addConfig(const char *option) {
	writeConfig(option, NULL, false);
}

void addConfigInt(const char *option, int value) {
	char temp[64];

	sprintf(temp, "%d", value);
	writeConfig(option, temp, false);
}

void addConfigDouble(const char *option, double value) {
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

bool checkNumber(const char *line) {
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

bool checkTimeValue(const char *line) {
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

bool empty(const char *line) {
	while (isspace(*line)) line++;
	return *line == 0;
}

bool checkString(const char *line) {
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

/**
 * This function returns the line, but removes all the comments.
 * note: it also removes resulting whitespace at beginning and end.
 */
char* stripComments(char* line) {
	unsigned start = 0;
	char* comment;

	if (line == NULL || strlen(line) == 0) {
		return line;
	}

	//printf("Stripping: `%s'\n", line);

	// find start
	while (line[start] == ' ' || line[start] == '\t')
		start++;

	// find start of comment
	comment = strstr(line, "//");
	if (comment == NULL) {
		int i = strlen(line) - 1;
		while (i > 0 && (line[i] == '\n' || 
				 line[i] == ' '  ||
				 line[i] == '\t' )) {
			line[i] = '\0';
			i--;
		}
		return line + start;
	}

	*comment = '\0';
	return line + start;
}

/**
 * returns the comments if any
 */
char* getComments(char* line) {
	char * comment;
	if (line == NULL || strlen(line) == 0)
		return NULL;
	
	// find start of comment
	comment = strstr(line, "//");
	return comment ? stripComments(comment + 2) : NULL;
}

/**
 * returns the name of the parameter
 * note: this assumes a stripped line
 * note: this returns a newly malloced string
 */
char* getParamName(char* line) {
	size_t colIndex;
	char* name;

	if (line == NULL || strlen(line) == 0)
		return NULL;

	colIndex = strcspn(line, ":");

	name = malloc(colIndex + 1);
	if (name == NULL) {
		printf("Out of memory in getParamName()\n");
		exit(EXIT_FAILURE);
	}

	strncpy(name, line, colIndex);
	name[colIndex] = '\0';
	return name;
}

/**
 * returns the type of the parameter
 * note: this assumes a stripped line
 */
paramType getParamType(char* line) {
	char* type;
	char* comment;

	if (line == NULL || strlen(line) == 0)
		return INVALID_PARAMTYPE;

	comment = strstr(line, ":");
	if (!comment) {
		printf("Expected ':' in \"%s\"\n", line);
		return INVALID_PARAMTYPE;
	}
	type = stripComments(comment + 1);
	if (strncmp(type, "numeric", 7) == 0)
		return NUMERIC;
	if (strncmp(type, "const numeric", 13) == 0)
		return NUMERIC_CONST;
	if (strncmp(type, "string", 6) == 0)
		return STRING;
	if (strncmp(type, "bool", 4) == 0)
		return BOOL;
	if (strncmp(type, "char", 4) == 0)
		return CHAR;
	if (strncmp(type, "anytype", 7) == 0)
		return ANY;

	// fallthrough case
	printf("Encountered unknown parameter type: %s\n", type);
	return INVALID_PARAMTYPE;
}

Module* _findModules(char* dir) {
	int result;
	int i = 0;
	size_t lineLength;
	ssize_t read = 0;
	FILE* file;
	char* lineContents;
	char* fileString = (char*)malloc(1024);
	struct dirent** baseFiles;
	Module* firstModule = NULL;
	Module* lastModule = NULL;

	if (fileString == NULL) {
		printf("Memory trouble\n");
		return NULL;	
	}

	// looking for base modules
	result = scandir(dir, &baseFiles, nedCheck, alphasort);
	printf("Found %d files in %s\n", result, dir);

	while (result-- > 0) {
		//int searchRes;
		bool foundParamSection = false;

		sprintf(fileString, "%s%s", dir, baseFiles[i++]->d_name);
		printf("Reading %s\n", fileString);
		file = fopen(fileString, "r");
		read = 0;
		if (file == NULL) {
			perror("Couldn't read file");
		} else {
			// found a module
			Module* mod = malloc(sizeof(Module));
			Parameter* lastParam = NULL;
			char* fName;

			if (mod == NULL) {
				printf("Memory trouble\n");
				free(fileString);
				return NULL;
			}
			
			mod->name = malloc(100);
			mod->next = NULL;
			mod->depends = NULL;
			mod->type = INVALID_MODULETYPE;

			if (firstModule == NULL) {
				firstModule = mod;
				lastModule = mod;
			} else {
				lastModule->next = mod;
				lastModule = mod;
			}
			
			fName = basename(fileString);
			mod->name = strncpy(mod->name, fName, strlen(fName) - 4);
			mod->name[strlen(fName) - 4] = '\0';
			mod->parameters = NULL;

			while (read != -1) {
				lineContents = NULL;
				lineLength = 0;
				read = getline(&lineContents, &lineLength, file);
				// find the parameters
				if (!foundParamSection) {
					if (strstr(lineContents, "parameters:") != NULL) {
						foundParamSection = true;
					}
				} else {
					Parameter* param = malloc(sizeof(Parameter));
					char* par;
					if (param == NULL) {
						printf("Memory trouble\n");
						free(fileString);
						return NULL;
					}

					param->comment = getComments(lineContents);
					param->next = NULL;

					par = stripComments(lineContents);

					param->name = getParamName(par);
					param->type = getParamType(par);
					printf("Found param: %s of type %d (%s)\n", param->name, param->type, param->comment);

					if (mod->parameters == NULL) {
						mod->parameters = param;
						lastParam = param;
					} else {
						lastParam->next = param;
						lastParam = param;
					}

					if (par[strlen(par) - 1] == ';') {
						read = -1; // dirty way to stop reading this file and continue with next
					}
				}

/*				searchRes = rx.search(lineContents);
				if (searchRes != -1) {
					captured = rx.cap(0);
					//printf("Found component: %s is a %s module\n", rx.cap(1).latin1(), rx.cap(2).latin1());
					map.insert(pair<const char* const, const char* const>(strdup(rx.cap(2).latin1()), strdup(rx.cap(1).latin1())));
				}*/
			}
			fclose(file);
		}
	}
	// TODO 
	// add the base module to this list
	free(fileString);
	return firstModule;
}

Module* findBaseModules() {
	int result;
	char* sourceDirString = (char*)malloc(1024);
	Module* mod;

	if (sourceDirString == NULL) {
		printf("Memory trouble\n");
		return NULL;	
	}

	result = snprintf(sourceDirString, 1023, "%s/base/modules/", sourcePath);
	if (result < 0 || result > 1024) {
		printf("Dir length trouble\n");
		free (sourceDirString);
		return NULL;
	}

	mod = _findModules(sourceDirString);
	free (sourceDirString);
	return mod;
}

Module* findModules(moduleType type) {
	int result;
	char* sourceDirString = (char*)malloc(1024);
	char* typeString;
	Module* mod;

	if (sourceDirString == NULL) {
		printf("Memory trouble\n");
		return NULL;	
	}

	switch (type) {
		case APPLICATION:
			typeString = "application";
			break;
		case NODE:
			typeString = "node";
			break;
		case NETWORK:
			typeString = "netw";
			break;
		case MAC:
			typeString = "mac";
			break;
		case PHY:
			typeString = "phy";
			break;
		case BATTERY:
			typeString = "battery";
			break;
		case MOBILITY:
			typeString = "mobility";
			break;
		case ARP:
			typeString = "ARP";
			break;
		case UTILITY:
			typeString = "utility";
			break;
		default:
			typeString = "ERROR";
			printf("Unknown type %d detected!\n", type);
	}

	printf("Looking for %s modules...\n", typeString);

	result = snprintf(sourceDirString, 1023, "%s/modules/%s/", sourcePath, typeString);
	if (result < 0 || result > 1024) {
		printf("Dir length trouble\n");
		free (sourceDirString);
		return NULL;
	}

	mod = _findModules(sourceDirString);
	free (sourceDirString);
	return mod;
}

