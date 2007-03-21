INCLUDES = -I$(top_srcdir)/. -I$(top_srcdir)/kgame-patch -I$(top_srcdir)/ksirk \
	-I$(top_srcdir)/ksirk/Dialogs -I$(top_srcdir)/ksirk/GameLogic -I.. $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libksirk_gamelogic.la
libksirk_gamelogic_la_LDFLAGS = $(all_libraries)
libksirk_gamelogic_la_SOURCES = player.cpp onu.cpp dice.cpp gameautomaton.cpp \
						aiplayer.cpp continent.cpp country.cpp nationality.cpp aiplayerio.cpp \
			eventslistproperty.cpp kstringvector.cpp goal.cpp
noinst_HEADERS = gameautomaton.h aiplayerio.h eventslistproperty.h \
		KMessageParts.h goal.h
