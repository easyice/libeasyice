## Process this file with automake to produce Makefile.in

SUBDIRS = src examples misc
DIST_SUBDIRS = $(SUBDIRS) doc wince

EXTRA_DIST = libdvbpsi.spec libdvbpsi.spec.in libdvbpsi.pc.in bootstrap

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdvbpsi.pc

AUTOMAKE_OPTIONS = foreign dist-bzip2 subdir-objects

doc-dummy:

doc: doc-dummy
	$(MAKE) -C doc doc

test_dr.c:
	$(MAKE) -C misc test_dr.c

changelog:
	cvs2cl --utc --hide-filenames --no-wrap -w --stdout -g -z9 | \
	  sed -e 's/^[^0-9]/ /' -e 's/^  *$$//' -e 's/^ \* 	/ /g' | \
	  grep -v '^ \*$$' | uniq >| ChangeLog

