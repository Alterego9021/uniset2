if HAVE_TESTS

noinst_PROGRAMS = tests

tests_SOURCES   = NullSM.cc TestObject_SK.h TestObject_SK.cc TestObject.cc tests.cc test_sm.cc
tests_LDADD	 = $(top_builddir)/lib/libUniSet2.la $(top_builddir)/extensions/lib/libUniSet2Extensions.la \
	$(top_builddir)/extensions/SharedMemory/libUniSet2SharedMemory.la \
	$(SIGC_LIBS) $(COMCPP_LIBS)
tests_CPPFLAGS  = -I$(top_builddir)/include -I$(top_builddir)/extensions/include \
	-I$(top_builddir)/extensions/SharedMemory $(SIGC_CFLAGS)  $(COMCPP_CFLAGS)

include $(top_builddir)/testsuite/testsuite-common.mk

check-local: atconfig package.m4 $(TESTSUITE) sm-tests.at
	$(SHELL) $(TESTSUITE) $(TESTSUITEFLAGS)

clean-local:
	rm -rf $(CLEANFILES)
	rm -rf $(COVERAGE_REPORT_DIR)
	rm -rf *_SK.*

TestObject_SK.cc TestObject.h: testobject.src.xml
	$(UNISET_CODEGEN) -l $(top_builddir)/Utilities/codegen -n TestObject --topdir $(top_builddir)/ --ask --no-main testobject.src.xml

include $(top_builddir)/include.mk

endif
