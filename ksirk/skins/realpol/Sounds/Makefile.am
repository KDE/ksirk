####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = canon.wav crash.wav roule.wav 

default_skin_dir = $(kde_datadir)/ksirk/skins/default

install-data-local:
	$(mkinstalldirs) $(default_skin_dir)/Sounds
	$(INSTALL_DATA) $(srcdir)/canon.wav $(default_skin_dir)/Sounds/canon.wav
	$(mkinstalldirs) $(default_skin_dir)/Sounds
	$(INSTALL_DATA) $(srcdir)/crash.wav $(default_skin_dir)/Sounds/crash.wav
	$(mkinstalldirs) $(default_skin_dir)/Sounds
	$(INSTALL_DATA) $(srcdir)/roule.wav $(default_skin_dir)/Sounds/roule.wav

uninstall-local:
	-rm -f $(default_skin_dir)/Sounds/canon.wav
	-rm -f $(default_skin_dir)/Sounds/crash.wav
	-rm -f $(default_skin_dir)/Sounds/rain.mid
	-rm -f $(default_skin_dir)/Sounds/roule.wav

####### kdevelop will overwrite this part!!! (end)############
