AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EKANJI_CFLAGS@

bin_PROGRAMS = ekanji

ekanji_INCLUDES = \
ekanji_canvas.h

ekanji_SOURCES = \
main.c \
ekanji_canvas.c

ekanji_LDADD = -lzinnia @EKANJI_LIBS@
