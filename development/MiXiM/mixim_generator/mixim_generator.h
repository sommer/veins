#ifndef __GENERATOR_H__
#define __GENERATOR_H__

/*
#include <termios.h>
#include <grp.h>
#include <pwd.h>
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef __WIN32__
#include <argp.h>
#include <stdbool.h>
#endif

#include <sys/types.h>

#ifndef __WIN32__
#include "config.h"
#endif

#define BUFSIZE	256

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define textdomain(Domain)
# define _(Text) Text
#endif
#define N_(Text) Text

#ifndef __WIN32__

/* argp option keys */
enum {
	DUMMY_KEY = 129,
	DIRECTORY_KEY
};
#endif
#ifdef __cplusplus
extern "C" {
#endif
void init(const char* path);
void finish(void);

void addConfig(char *option);
void addConfigInt(char *option, int value);
void addConfigString(char *option, const char *value);
void stripNewline(char *line);
bool checkTimeValue(char *line);
void writeConfig(char *option, const char *value, bool quoted);
bool empty(char *line);
bool checkNumber(char *line);
bool checkString(char *line);
void writeNode(unsigned nodeID, double x, double y, double z);
#ifdef __cplusplus
}
#endif

#endif

