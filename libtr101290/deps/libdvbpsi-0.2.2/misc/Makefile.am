## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = gen_crc gen_pat gen_pmt \
                  test_dr


gen_crc_SOURCES = gen_crc.c

gen_pat_SOURCES = gen_pat.c
gen_pat_LDFLAGS = -L../src -ldvbpsi

gen_pmt_SOURCES = gen_pmt.c
gen_pmt_LDFLAGS = -L../src -ldvbpsi


test_dr_SOURCES = test_dr.c
test_dr_LDFLAGS = -L../src -ldvbpsi

noinst_HEADERS = test_dr.h

EXTRA_DIST=dr.dtd dr.xml dr.xsl

test_dr.c: dr.dtd dr.xml dr.xsl
	xsltproc -o test_dr.c dr.xsl dr.xml

