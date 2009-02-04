AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ENSCRIBI_CFLAGS@

bin_PROGRAMS = enscribi

enscribi_INCLUDES = \
enscribi_util.h \
enscribi_recognizer.h \
enscribi_canvas.h \
enscribi_input_frame.h

enscribi_SOURCES = \
main.c \
enscribi_util.c \
enscribi_recognizer.c \
enscribi_canvas.c \
enscribi_input_frame.c

enscribi_LDADD = -lzinnia @ENSCRIBI_LIBS@
