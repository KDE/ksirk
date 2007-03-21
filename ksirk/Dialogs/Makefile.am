INCLUDES = -I$(top_srcdir)/. -I$(top_srcdir)/ksirk \
	-I$(top_srcdir)/ksirk/Dialogs -I$(top_srcdir)/kgame-patch $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libksirk_dialogs.la
libksirk_dialogs_la_LDFLAGS = $(all_libraries)
libksirk_dialogs_la_SOURCES = newGameDialog.ui qplayersetupdialog.ui \
qwaitedplayersetupdialog.ui newGameDialogImpl.cpp kplayersetupdialog.cpp \
	kwaitedplayersetupdialog.cpp
noinst_HEADERS = kwaitedplayersetupdialog.h qwaitedplayersetupdialog.h
