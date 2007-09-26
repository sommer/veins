TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

FORMS	= generatorwizard.ui \
	node.ui

HEADERS += main.h GeneratorWizard.h Node.h

SOURCES	+= main.cpp GeneratorWizard.cpp Node.cpp \
	../miximConfiguratorCommon.c

IMAGES	= grid.xpm \
	random.xpm

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



#The following line was inserted by qt3to4
QT +=  qt3support 

