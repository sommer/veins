TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp \
	../miximConfiguratorCommon.c

FORMS	= generatorwizard.ui \
	node.ui

IMAGES	= grid.xpm \
	random.xpm

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



