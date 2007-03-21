INCLUDES = -I$(top_srcdir)/kgame-patch -I$(top_srcdir)/ksirk \
	-I$(top_srcdir)/ksirk/Sprites $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libksirksprites.la
libksirksprites_la_LDFLAGS = $(all_libraries)
libksirksprites_la_SOURCES = animsprite.cpp animspriteslist.cpp \
	cannonsprite.cpp cavalrysprite.cpp infantrysprite.cpp skinSpritesData.cpp
noinst_HEADERS = armysprite.h skinSpritesData.h
