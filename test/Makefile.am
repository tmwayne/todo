AM_TESTSUITE_SUMMARY_HEADER = ' of unit tests for $(PACKAGE_STRING)'

TESTS = $(check_PROGRAMS)
check_PROGRAMS = test_prototype

test_prototype_SOURCES = test-prototype.c
test_prototype_LDADD = ../src/common/libcommon.la

AM_CPPFLAGS = -I$(top_srcdir)/include
