bin_PROGRAMS = todo
todo_SOURCES = todo.c \
	view.c
todo_CPPFLAGS = -I$(top_srcdir)/include
todo_LDADD = $(top_srcdir)/src/common/libcommon.la  $(top_srcdir)/src/backend/libbackend.la
todo_LDFLAGS = -static
