####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = onu.xml 

default_skin_dir = $(kde_datadir)/ksirk/skins/default

install-data-local:
	$(mkinstalldirs) $(default_skin_dir)/Data
	$(INSTALL_DATA) $(srcdir)/onu.xml $(default_skin_dir)/Data/onu.xml

uninstall-local:
	-rm -f $(default_skin_dir)/Data/onu.xml

####### kdevelop will overwrite this part!!! (end)############
