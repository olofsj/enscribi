AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ENSCRIBI_CFLAGS@

bin_PROGRAMS = enscribi enscribi_recognizer_exe

enscribi_INCLUDES = \
enscribi_util.h \
enscribi_recognizer.h \
enscribi_recognizer_exe.h \
enscribi_canvas.h \
enscribi_input_frame.h

enscribi_SOURCES = \
main.c \
enscribi_util.c \
enscribi_recognizer.c \
enscribi_canvas.c \
enscribi_input_frame.c

enscribi_LDADD = @ENSCRIBI_LIBS@

enscribi_recognizer_exe_INCLUDES = \
enscribi_recognizer_exe.h 

enscribi_recognizer_exe_SOURCES = \
enscribi_recognizer_exe.c 

enscribi_recognizer_exe_LDADD = -lzinnia @ENSCRIBI_LIBS@
