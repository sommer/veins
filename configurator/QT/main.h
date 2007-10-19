#include <math.h>
#include <time.h>

#include <QApplication>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include <dirent.h>
#include <libgen.h>

#include <map>

#include "../miximConfiguratorCommon.h"
#include "GeneratorWizard.h"
#include "Node.h"

using namespace std;

struct ltstr {
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

typedef multimap<const char *, const char *, ltstr> modules;
typedef QMap<QString, QString> ComponentMap;

