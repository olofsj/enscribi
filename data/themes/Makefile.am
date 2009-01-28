MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = images

EDJE_CC = edje_cc
EDJE_FLAGS = \
-v \
-id $(top_srcdir)/data/themes/images \
-fd $(top_srcdir)/data/themes/fonts 

filesdir = $(datadir)/$(PACKAGE)/themes
files_DATA = \
ekanji.edj

EXTRA_DIST = \
ekanji.edc

ekanji.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/ekanji.edc \
	$(top_builddir)/data/themes/ekanji.edj

clean-local:
	rm -f *.edj
