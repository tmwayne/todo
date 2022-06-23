bin_PROGRAMS = todo
todo_SOURCES = dump.c \
	edit.c \
	import.c \
	todo.c \
	view.c
todo_CPPFLAGS = -I$(top_srcdir)/include
todo_LDADD = $(top_srcdir)/src/common/libcommon.la  \
	$(top_srcdir)/src/backend-sqlite3/libbackend.la \
	$(top_srcdir)/src/backend-delim/libreader.la
todo_LDFLAGS = -static
