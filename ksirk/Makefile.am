####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = ksirk

## INCLUDES were found outside kdevelop specific part

ksirk_SOURCES = kgamewin.cpp gestionSprites.cpp decoratedgameframe.cpp \
		backgnd.cpp main.cpp kgamewinslots.cpp
ksirk_LDADD = $(top_builddir)/ksirk/Sprites/libksirksprites.la \
	$(top_builddir)/ksirk/SaveLoad/libksirk_saveload.la $(top_builddir)/ksirk/GameLogic/libksirk_gamelogic.la \
	$(top_builddir)/ksirk/Dialogs/libksirk_dialogs.la -lcrypt -lkdegames -lkio -lpthread -lkdeui -lqt-mt -lkdecore

SUBDIRS = Dialogs Sprites GameLogic SaveLoad skins

EXTRA_DIST = main.cpp ksirk.desktop lo32-app-ksirk.png lo16-app-ksirk.png aiplayer.cpp aiplayer.h animsprite.cpp animsprite.h animspriteslist.cpp animspriteslist.h backgnd.cpp backgnd.h continent.cpp continent.h country.cpp country.h decoratedgameframe.cpp decoratedgameframe.h gestionSprites.cpp global.h hi16-app-ksirk.png hi32-app-ksirk.png hi48-app-ksirk.png kgamewin.cpp kgamewin.h kplayersetupdialog.cpp kplayersetupdialog.h nationality.cpp nationality.h onu.cpp onu.h player.cpp player.h dice.cpp dice.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/ksirk.desktop $(kde_appsdir)/Applications/ksirk.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-ksirk.png $(kde_icondir)/locolor/32x32/apps/ksirk.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-ksirk.png $(kde_icondir)/locolor/16x16/apps/ksirk.png
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) $(srcdir)/hi16-app-ksirk.png $(kde_icondir)/hi16-app-ksirk.png
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) $(srcdir)/hi32-app-ksirk.png $(kde_icondir)/hi32-app-ksirk.png
	$(mkinstalldirs) $(kde_icondir)/
	$(INSTALL_DATA) $(srcdir)/hi48-app-ksirk.png $(kde_icondir)/hi48-app-ksirk.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/ksirk.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/ksirk.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/ksirk.png
	-rm -f $(kde_icondir)/hi16-app-ksirk.png
	-rm -f $(kde_icondir)/hi32-app-ksirk.png
	-rm -f $(kde_icondir)/hi48-app-ksirk.png

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use kde_icondir.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use kde_icondir).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/kgame-patch -I$(top_srcdir)/ksirk -IDialogs \
	-I$(top_srcdir)/libkdegames $(all_includes)

METASOURCES = AUTO

# the library search path. 


# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDEs XML GUI builing
#rcdir = $(kde_datadir)/ksirk
#rc_DATA = ksirkui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/ksirk.pot; \
	fi



