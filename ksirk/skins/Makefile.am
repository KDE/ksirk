INCLUDES = -I$(top_srcdir)/ksirk -I$(top_srcdir)/ksirk/GameLogic -I$(top_srcdir)/ksirk/Sprites $(all_includes)
METASOURCES = AUTO
SUBDIRS = default bubble

skins_dir = $(kde_datadir)/ksirk/skins

install-data-local:
	$(mkinstalldirs) $(skins_dir)
	$(INSTALL_DATA) $(srcdir)/skinsdir $(skins_dir)/skinsdir
