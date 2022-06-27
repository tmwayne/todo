BUILT_SOURCES = parser.h
AM_YFLAGS = -d -p "cr_yy"

noinst_LTLIBRARIES = libconfigreader.la
# TODO: lexer.l has to be built first because config-reader.c
# depends on lexer.h and Automake doesn't know about lexer.h
libconfigreader_la_SOURCES = lexer.l \
	config-reader.c \
	parser.y
libconfigreader_la_CPPFLAGS = -I$(top_srcdir)/include \
	-I$(top_srcdir)/src/config-reader
