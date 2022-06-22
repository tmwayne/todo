BUILT_SOURCES = parser.h
AM_YFLAGS = -d

noinst_LTLIBRARIES = libreader.la
libreader_la_SOURCES = reader.c \
	parser.y \
	lexer.c
libreader_la_CPPFLAGS = -I$(top_srcdir)/include
