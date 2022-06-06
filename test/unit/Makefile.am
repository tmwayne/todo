AM_TESTSUITE_SUMMARY_HEADER = ' of unit tests for $(PACKAGE_STRING)'

TESTS = $(check_PROGRAMS)
check_PROGRAMS = test_prototype

test_prototype_SOURCES = test-prototype.c \
	$(top_srcdir)/src/common/task.c \
	$(top_srcdir)/src/common/error-functions.c \
	$(top_srcdir)/src/backend/backend-sqlite3.c
# test_prototype_LDADD = $(top_srcdir)/src/common/libcommon.la
test_prototype_CFLAGS = -DTESTING

AM_CPPFLAGS = -I$(top_srcdir)/include
