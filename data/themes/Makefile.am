MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = images

EDJE_CC = edje_cc
EDJE_FLAGS = \
-v \
-id $(top_srcdir)/data/themes/images \
-fd $(top_srcdir)/data/themes/fonts 

filesdir = $(datadir)/$(PACKAGE)/themes
files_DATA = \
enscribi.edj

EXTRA_DIST = \
enscribi.edc

enscribi.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/enscribi.edc \
	$(top_builddir)/data/themes/enscribi.edj

clean-local:
	rm -f *.edj
