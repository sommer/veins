TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp \
	../mixim_generator.c

FORMS	= generatorwizard.ui

IMAGES	= grid.xpm \
	random.xpm

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



