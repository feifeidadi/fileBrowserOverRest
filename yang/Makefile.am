CONFDC       = $(CONFD_DIR)/bin/confdc

BUILT_SOURCES = my-fileBrowser.fxs my-fileBrowser.h

%.h: %.fxs
	$(CONFDC) --emit-h $*.h $*.fxs
	-cp -a *.h ../inc

%.fxs: %.yang
	$(CONFDC) -c -o $@  $<

CLEANFILES = *.h  *.fxs
